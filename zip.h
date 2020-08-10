#pragma once
#include <tuple>
#include <utility>

namespace zip_impl {

    template<typename...>
    class ZipIterator;

    template <typename ... Elements>
    class Tuple;

    template <typename T>
    struct IsZipTuple : public std::false_type {};

    template <typename ... Elements>
    struct IsZipTuple<Tuple<Elements...>> : public std::true_type {};

    template <typename ... Elements>
    struct Tuple {
        static_assert(std::conjunction_v<
                std::disjunction< std::is_reference<Elements>, IsZipTuple<Elements> >...
        >);

        using Base = std::tuple<Elements...>;
        Base base;

        Tuple(const Base& b) : base(b) {}
        Tuple(Base&& b) : base(move(b)) {}
        explicit Tuple(Elements&&... elem) : base(std::forward<Elements>(elem)...) {}

        void swap(Tuple& other) {
            using std::swap;
            swap(base, other.base);
        }

        template <size_t Index>
        auto& get() const {
            return std::get<Index>(base);
        }

        //operator const Base&() const { return base; }
    };

    template<typename Iterator>
    struct value_helper {
        using value = typename std::iterator_traits<Iterator>::reference;
        using const_value = const typename std::iterator_traits<Iterator>::value_type&;
    };

    template<typename... Iterators>
    struct value_helper<ZipIterator<Iterators...>> {
        using value = typename ZipIterator<Iterators...>::value_type;
        // using const_value = typename ZipIterator<Iterators...>::const_value_type;
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
        using value_type = Tuple<typename value_helper<Iters>::value...>;
        using difference_type = int;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        template<typename F, size_t... Indexes>
        inline void ApplyToIterators(F&& f, std::integer_sequence<size_t, Indexes...>) {
            ((f(std::get<Indexes>(*this))), ...);
        }

        template<bool default_value, typename F, size_t... Indexes>
        inline bool AnyPair(F&& f, const ZipIterator& other, std::integer_sequence<size_t, Indexes...>) const {
            if constexpr (sizeof...(Indexes) != 0)
                return (... || f(std::get<Indexes>(*this), std::get<Indexes>(other)));
            else
                return default_value;
        }

        template<size_t... Indexes>
        inline value_type CombineValues(std::integer_sequence<size_t, Indexes...>) {
            return value_type(*std::get<Indexes>(*this)...);
        }

    public:
        ZipIterator& operator++() {
            ApplyToIterators([](auto& x){ ++x; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::bidirectional_iterator_tag>, int>>
        ZipIterator& operator--() {
            ApplyToIterators([](auto& x){ --x; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        value_type operator*() {
            return CombineValues(std::index_sequence_for<Iters...>{});
        }

        bool operator==(const ZipIterator& other) const {
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
            return AnyPair<true>([](const auto& it1, const auto& it2){ return it1==it2; }, other, std::index_sequence_for<Iters...>{});
        }

        inline bool operator!=(const ZipIterator& other) const {
            return !operator==(other);
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        ZipIterator& operator+=(int n) {
            ApplyToIterators([n](auto& it) { it += n; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        ZipIterator operator+(int n) {
            ZipIterator copy = *this;
            return copy += n;
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        ZipIterator& operator-=(int n) {
            ApplyToIterators([n](auto& it) { it -= n; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        ZipIterator operator-(int n) {
            ZipIterator copy = *this;
            return copy -= n;
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        difference_type operator-(const ZipIterator& other) const {
            static_assert(sizeof...(Iters) != 0);
            return std::get<0>(*this) - std::get<0>(other);
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        bool operator>(const ZipIterator& other) {
            return AnyPair<false>([](const auto& it1, const auto& it2){ return it1 > it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        bool operator<(const ZipIterator& other) {
            return AnyPair<false>([](const auto& it1, const auto& it2){ return it1 < it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        bool operator>=(const ZipIterator& other) {
            return AnyPair<false>([](const auto& it1, const auto& it2){ return it1 >= it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        bool operator<=(const ZipIterator& other) {
            return AnyPair<false>([](const auto& it1, const auto& it2){ return it1 <= it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = std::enable_if_t<std::is_convertible_v<iterator_category, std::random_access_iterator_tag>, int>>
        value_type operator[](size_t index) {
            return *(*this + index);
        }

        void Swap(ZipIterator& other) {
            Base::swap(other);
        }

        inline const Base& AsTuple() const { return *this; }
    };

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

    template <typename ... Types>
    void swap(ZipIterator<Types...>& it1, ZipIterator<Types...>& it2) {
        it1.Swap(it2);
    }

    template <typename ... Elements>
    void swap(const Tuple<Elements...>& lhs, const Tuple<Elements...>& rhs) {
        using std::swap;
        swap(lhs.base, rhs.base);
    }

    template <typename ... Elements>
    void swap(Tuple<Elements...>&& lhs, Tuple<Elements...>&& rhs) {
        using std::swap;
        swap(lhs.base, rhs.base);
    }

    template <size_t Index, typename ... Elements>
    auto& get(const Tuple<Elements...>& tup) {
        return tup.template get<Index>();
    }

    template <typename ... Args1, typename ... Args2>
    bool operator==(const Tuple<Args1...>& lhs, const Tuple<Args2...>& rhs) {
        return lhs.base == rhs.base;
    }

    template <typename ... Args1, typename ... Args2>
    bool operator==(const std::tuple<Args1...>& lhs, const Tuple<Args2...>& rhs) {
        return lhs == rhs.base;
    }

    template <typename ... Args1, typename ... Args2>
    bool operator==(const Tuple<Args1...>& lhs, const std::tuple<Args2...>& rhs) {
        return lhs.base == rhs;
    }

    template <typename ... Args1, typename ... Args2>
    bool operator<(const Tuple<Args1...>& lhs, const Tuple<Args2...>& rhs) {
        return lhs.base < rhs.base;
    }
}

template<typename... Types>
zip_impl::Zip<Types...> zip(Types&& ... args) {
    return zip_impl::Zip<Types...>(std::forward<Types>(args)...);
}

namespace std {
    template <typename ... Types>
    struct tuple_size<zip_impl::Tuple<Types...>> : public std::integral_constant<size_t, sizeof...(Types)> {};

    template <size_t Index, typename ... Types>
    struct tuple_element<Index, zip_impl::Tuple<Types...>> : public tuple_element<Index, std::tuple<Types...>> {};
}
