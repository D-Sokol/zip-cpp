#pragma once
#include <tuple>
#include <utility>

namespace zip_impl {

    template<typename...>
    class ZipIterator;
    template<typename...>
    class ConstZipIterator;

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

        Tuple(const Tuple&) = default;
        Tuple(Tuple&&) noexcept = default;
        Tuple(const Base& b) : base(b) {}
        Tuple(Base&& b) noexcept : base(std::move(b)) {}
        template <typename ... UElements, typename = std::enable_if<std::conjunction_v<std::is_constructible<Elements, UElements>...>, int>>
        explicit Tuple(UElements&&... elem) : base(std::forward<UElements>(elem)...) {}

        Tuple& operator=(Tuple&&) noexcept = default;
        Tuple& operator=(const Tuple&) = default;
        template <typename U>
        Tuple& operator=(U&& other) {
            base = std::forward<U>(other);
            return *this;
        }

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
        using const_value = typename ConstZipIterator<Iterators...>::value_type;
    };

    template<typename... Iterators>
    struct value_helper<ConstZipIterator<Iterators...>> {
        using value = typename ConstZipIterator<Iterators...>::value_type;
        using const_value = typename ConstZipIterator<Iterators...>::value_type;
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
    class BaseZipIterator : protected std::tuple<Iters...> {
    public:
        using Base = std::tuple<Iters...>;
        using Base::Base;
        BaseZipIterator(const Base& base) : Base(base) {}
        BaseZipIterator(Base&& base) : Base(std::move(base)) {}

        using iterator_category = typename category_helper<Iters...>::type;
        static_assert(std::is_convertible_v<iterator_category, std::input_iterator_tag>);
    protected:
        template <typename required_tag>
        using minimal_category = std::enable_if_t<std::is_convertible_v<iterator_category, required_tag>, int>;

        template<typename F, size_t... Indexes>
        inline void ApplyToIterators(F&& f, std::integer_sequence<size_t, Indexes...>) {
            ((f(std::get<Indexes>(*this))), ...);
        }

        template<typename Self, bool default_value, typename F, size_t... Indexes>
        inline bool AnyPair(F&& f, const Self& other, std::integer_sequence<size_t, Indexes...>) const {
            if constexpr (sizeof...(Indexes) != 0)
                return (... || f(std::get<Indexes>(*this), std::get<Indexes>(other)));
            else
                return default_value;
        }

        template<typename Value, size_t... Indexes>
        inline Value CombineValues(std::integer_sequence<size_t, Indexes...>) {
            return Value(*std::get<Indexes>(*this)...);
        }
    public:
        inline const Base& AsTuple() const { return *this; }
    };

    template<typename... Iters>
    class ZipIterator : public BaseZipIterator<Iters...> {
    public:
        using BaseZipIterator<Iters...>::BaseZipIterator;
        ZipIterator(const typename BaseZipIterator<Iters...>::Base& base) : BaseZipIterator<Iters...>(base) {}
        ZipIterator(typename BaseZipIterator<Iters...>::Base&& base) : BaseZipIterator<Iters...>(std::move(base)) {}

        using Self = ZipIterator<Iters...>;

        using value_type = Tuple<typename value_helper<Iters>::value...>;
        using difference_type = int;
        using pointer = value_type*;
        using reference = value_type&;
    private:
        template <typename required_tag>
        using minimal_category = typename BaseZipIterator<Iters...>::template minimal_category<required_tag>;

    public:
        Self& operator++() {
            this->ApplyToIterators([](auto& x){ ++x; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        Self operator++(int) {
            auto it = *this;
            ++(*this);
            return it;
        }

        template <typename = minimal_category<std::bidirectional_iterator_tag>>
        Self& operator--() {
            this->ApplyToIterators([](auto& x){ --x; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        template <typename = minimal_category<std::bidirectional_iterator_tag>>
        Self operator--(int) {
            auto it = *this;
            --(*this);
            return it;
        }

        value_type operator*() {
            return this->template CombineValues<value_type>(std::index_sequence_for<Iters...>{});
        }

        bool operator==(const Self& other) const {
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
            return this->template AnyPair<Self, true>([](const auto& it1, const auto& it2){ return it1==it2; }, other, std::index_sequence_for<Iters...>{});
        }

        inline bool operator!=(const Self& other) const {
            return !operator==(other);
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        Self& operator+=(int n) {
            this->ApplyToIterators([n](auto& it) { it += n; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        Self operator+(int n) const {
            auto copy = *this;
            return copy += n;
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        Self& operator-=(int n) {
            this->ApplyToIterators([n](auto& it) { it -= n; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        Self operator-(int n) const {
            auto copy = *this;
            return copy -= n;
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        difference_type operator-(const Self& other) const {
            static_assert(sizeof...(Iters) != 0);
            return std::get<0>(*this) - std::get<0>(other);
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        bool operator>(const Self& other) const {
            return this->template AnyPair<Self, false>([](const auto& it1, const auto& it2){ return it1 > it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        bool operator<(const Self& other) const {
            return this->template AnyPair<Self, false>([](const auto& it1, const auto& it2){ return it1 < it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        bool operator>=(const Self& other) const {
            return this->template AnyPair<Self, true>([](const auto& it1, const auto& it2){ return it1 >= it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        bool operator<=(const Self& other) const {
            return this->template AnyPair<Self, true>([](const auto& it1, const auto& it2){ return it1 <= it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        value_type operator[](size_t index) {
            return *(*this + index);
        }

        void Swap(Self& other) {
            std::tuple<Iters...>::swap(other);
        }
    };

    template<typename... Iters>
    class ConstZipIterator : public BaseZipIterator<Iters...> {
    public:
        using BaseZipIterator<Iters...>::BaseZipIterator;
        ConstZipIterator(const typename BaseZipIterator<Iters...>::Base& base) : BaseZipIterator<Iters...>(base) {}
        ConstZipIterator(typename BaseZipIterator<Iters...>::Base&& base) : BaseZipIterator<Iters...>(std::move(base)) {}

        using Self = ConstZipIterator<Iters...>;

        using value_type = Tuple<typename value_helper<Iters>::const_value...>;
        using difference_type = int;
        using pointer = value_type*;
        using reference = value_type&;
    private:
        template <typename required_tag>
        using minimal_category = typename BaseZipIterator<Iters...>::template minimal_category<required_tag>;

    public:
        Self& operator++() {
            this->ApplyToIterators([](auto& x){ ++x; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        Self operator++(int) {
            auto it = *this;
            ++(*this);
            return it;
        }

        template <typename = minimal_category<std::bidirectional_iterator_tag>>
        Self& operator--() {
            this->ApplyToIterators([](auto& x){ --x; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        template <typename = minimal_category<std::bidirectional_iterator_tag>>
        Self operator--(int) {
            auto it = *this;
            --(*this);
            return it;
        }

        value_type operator*() {
            return this->template CombineValues<value_type>(std::index_sequence_for<Iters...>{});
        }

        bool operator==(const Self& other) const {
            if (this == &other)
                return true;
            return this->template AnyPair<Self, true>([](const auto& it1, const auto& it2){ return it1==it2; }, other, std::index_sequence_for<Iters...>{});
        }

        inline bool operator!=(const Self& other) const {
            return !operator==(other);
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        Self& operator+=(int n) {
            this->ApplyToIterators([n](auto& it) { it += n; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        Self operator+(int n) const {
            auto copy = *this;
            return copy += n;
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        Self& operator-=(int n) {
            this->ApplyToIterators([n](auto& it) { it -= n; }, std::index_sequence_for<Iters...>{});
            return *this;
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        Self operator-(int n) const {
            auto copy = *this;
            return copy -= n;
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        difference_type operator-(const Self& other) const {
            static_assert(sizeof...(Iters) != 0);
            return std::get<0>(*this) - std::get<0>(other);
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        bool operator>(const Self& other) const {
            return this->template AnyPair<Self, false>([](const auto& it1, const auto& it2){ return it1 > it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        bool operator<(const Self& other) const {
            return this->template AnyPair<Self, false>([](const auto& it1, const auto& it2){ return it1 < it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        bool operator>=(const Self& other) const {
            return this->template AnyPair<Self, true>([](const auto& it1, const auto& it2){ return it1 >= it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        bool operator<=(const Self& other) const {
            return this->template AnyPair<Self, true>([](const auto& it1, const auto& it2){ return it1 <= it2; }, other, std::index_sequence_for<Iters...>{});
        }

        template <typename = minimal_category<std::random_access_iterator_tag>>
        value_type operator[](size_t index) {
            return *(*this + index);
        }

        void Swap(Self& other) {
            std::tuple<Iters...>::swap(other);
        }
    };

    template <typename ... Iters, typename = std::enable_if_t<std::is_convertible_v<typename ZipIterator<Iters...>::iterator_category, std::random_access_iterator_tag>, int>>
    inline auto operator+(int n, const ZipIterator<Iters...>& it) {
        return it + n;
    }

    template <typename ... Iters, typename = std::enable_if_t<std::is_convertible_v<typename ConstZipIterator<Iters...>::iterator_category, std::random_access_iterator_tag>, int>>
    inline auto operator+(int n, const ConstZipIterator<Iters...>& it) {
        return it + n;
    }

    template<typename... Types>
    class Zip {
    public:
        using iterator = ZipIterator<std::remove_reference_t<decltype(std::begin(std::declval<Types>()))>...>;
        using const_iterator = ConstZipIterator<std::remove_reference_t<decltype(std::begin(std::declval<Types>()))>...>;
    private:
        using stored_iterators_tuple = typename iterator::Base;
    public:
        explicit Zip(Types&& ... args);

        inline auto begin() { return iterator(begin_); }
        inline auto end() { return iterator(end_); }
        inline auto begin() const { return const_iterator(begin_); }
        inline auto end() const { return const_iterator(end_); }

        inline auto cbegin() const { return begin(); }
        inline auto cend() const { return end(); }
    private:
        stored_iterators_tuple begin_;
        stored_iterators_tuple end_;
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


namespace zipcpp {
    template<typename... Types>
    zip_impl::Zip<Types...> zip(Types&& ... args) {
        return zip_impl::Zip<Types...>(std::forward<Types>(args)...);
    }

    template <typename Iter>
    class IterRange {
        Iter begin_;
        Iter end_;
    public:
        IterRange(Iter begin, Iter end) : begin_(begin), end_(end) {}
        Iter begin() const { return begin_; }
        Iter end() const { return end_; }
    };
}

namespace std {
    template <typename ... Types>
    struct tuple_size<zip_impl::Tuple<Types...>> : public std::integral_constant<size_t, sizeof...(Types)> {};

    template <size_t Index, typename ... Types>
    struct tuple_element<Index, zip_impl::Tuple<Types...>> : public tuple_element<Index, std::tuple<Types...>> {};
}
