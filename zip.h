#pragma once
#include <tuple>
#include <utility>

namespace zip_impl {

    template<typename...>
    class ZipIterator;

    template<typename Iterator>
    struct value_helper {
        using value = typename std::iterator_traits<Iterator>::reference;
        using const_value = const typename std::iterator_traits<Iterator>::value_type&;
    };

    template<typename... Iterators>
    struct value_helper<ZipIterator<Iterators...>> {
        using value = typename ZipIterator<Iterators...>::value_type;
        using const_value = typename ZipIterator<Iterators...>::const_value_type;
    };

    template<typename... Iters>
    struct category_helper {
        using type = std::common_type_t<typename std::iterator_traits<Iters>::iterator_category...>;
    };

    template<>
    struct category_helper<> {
        using type = std::input_iterator_tag;
    };

    template<typename... Iters>
    class ZipIterator : private std::tuple<Iters...> {
        using Base = std::tuple<Iters...>;
    public:
        using Base::Base;
        using iterator_category = typename category_helper<Iters...>::type;
        static_assert(std::is_convertible_v<iterator_category, std::input_iterator_tag>);

        // Something like tuple<int&, int&>. Used as return value of non-constant version of operator*.
        using value_type = std::tuple<typename value_helper<Iters>::value...>;
        using difference_type = int;
        using pointer = value_type*;
        using reference = value_type&;

        // Something like tuple<const int&, const int&>. Used as return value of constant version of operator*.
        using const_value_type = std::tuple<typename value_helper<Iters>::const_value...>;

        ZipIterator& operator++();

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::bidirectional_iterator_tag>, int>>
        ZipIterator& operator--() {
            ApplyToIterators([](auto& x){ --x; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        value_type operator*();

        const_value_type operator*() const;

        bool operator==(const ZipIterator& other) const;

        inline bool operator!=(const ZipIterator& other) const {
            return !operator==(other);
        }

        inline const Base& AsTuple() const { return *this; }

    private:
        template<typename F, size_t... Indexes>
        inline void ApplyToIterators(F&& f, std::integer_sequence<size_t, Indexes...>) {
            ((f(std::get<Indexes>(*this))), ...);
        }

        template<size_t... Indexes>
        inline bool AnyEquals(const ZipIterator& other, std::integer_sequence<size_t, Indexes...>) const {
            if constexpr (sizeof...(Indexes) != 0)
                return (... || (std::get<Indexes>(*this) == std::get<Indexes>(other)));
            else
                // Так как вызов функции zip() без аргументов должен возвращать пустой диапазон,
                //  то в случае, когда список типов Types пуст, все итераторы должны считаться равными между собой.
                // Предыдущая ветка при пустом списке возвращает false, см. https://en.cppreference.com/w/cpp/language/fold
                return true;
        }

        template<size_t... Indexes>
        inline value_type CombineValues(std::integer_sequence<size_t, Indexes...>) {
            return std::forward_as_tuple(*std::get<Indexes>(*this)...);
        }

        template<size_t... Indexes>
        inline const_value_type CombineValues(std::integer_sequence<size_t, Indexes...>) const {
            return std::forward_as_tuple(*std::get<Indexes>(*this)...);
        }
    };

    template<typename... Types>
    ZipIterator<Types...>& ZipIterator<Types...>::operator++() {
        ApplyToIterators([](auto& x){ ++x; }, std::index_sequence_for<Types...>{});
        return *this;
    }

    template<typename... Types>
    typename ZipIterator<Types...>::value_type ZipIterator<Types...>::operator*() {
        return CombineValues(std::index_sequence_for<Types...>{});
    }

    template<typename... Types>
    typename ZipIterator<Types...>::const_value_type ZipIterator<Types...>::operator*() const {
        return CombineValues(std::index_sequence_for<Types...>{});
    }

    template<typename... Types>
    bool ZipIterator<Types...>::operator==(const ZipIterator& other) const {
        if (this == &other)
            return true;
        // Поскольку при сравнении итераторов, полученных из разных контейнеров,
        //  поведение программы не определено,то при сравнении двух ZipIterator рассматривается только случай,
        //  когда оба итератора z1, z2 были получены при помощи корректного вызова класса Zip.
        // В таком случае возможны следующие сценарии:
        // 1. Сравниваются два итератора, изначально полученные при помощи метода Zip::begin, и, возможно,
        //    несколько раз инкрементированные. Если количество инкрементов у z1 и z2 одинаково,
        //    то все хранимые итераторы на контейнеры совпадают, иначе все хранимые итераторы на контейнера различны.
        // 2. Сравниваются два итератора, полученные при помощи Zip::end.
        //    В этом случае все итераторы, хранящиеся в z1 и z2 получены при помощи вызова функции end от контейнера,
        //    поэтому все хранимые итераторы равны.
        // 3. Один из итераторов получен при помощи Zip::begin и, возможно, несколько раз инкрементирован,
        //    другой получен при помощи метода Zip:end.
        //    Так как итерация по возвращаемому значению zip должна завершиться,
        //    как только хотя бы один из хранимых итераторов достигает конца диапазона,
        //    то в этом случае ZipIterator должны считаться равными тогда и только тогда,
        //    когда хотя бы один из хранимых итераторов в z1 равен соответствующему хранимому итератору z2.
        // В любом из рассматриваемых случаев достаточно проверить, выполнено ли равенство хранимых итераторов
        //  хотя бы для одной пары.
        return AnyEquals(other, std::index_sequence_for<Types...>{});
    }

    template<typename... Types>
    class Zip {
    public:
        using iterator = ZipIterator<std::remove_reference_t<decltype(std::begin(std::declval<Types>()))>...>;

        explicit Zip(Types&& ... args);

        inline iterator begin() { return begin_; }

        inline iterator end() { return end_; }

    private:
        iterator begin_;
        iterator end_;
    };

    template<typename... Types>
    Zip<Types...>::Zip(Types&& ... args)
            : begin_(std::begin(args)...), end_(std::end(args)...) {
    }
}

template<typename... Types>
zip_impl::Zip<Types...> zip(Types&& ... args) {
    return zip_impl::Zip<Types...>(std::forward<Types>(args)...);
}
