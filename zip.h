#pragma once
#include <tuple>
#include <utility>

template <typename... Iters>
class ZipIterator : private std::tuple<Iters...> {
    using Base = std::tuple<Iters...>;
public:
    using Base::Base;
    using value_type = std::tuple<typename std::iterator_traits<Iters>::value_type...>;

    ZipIterator& operator++();
    value_type operator*();

    bool operator==(const ZipIterator& other) const;
    inline bool operator!=(const ZipIterator& other) const {
        return !operator==(other);
    }

    inline const Base& AsTuple() const { return *this; }
private:
    // TODO: apply here the same trick as below (integer_sequence_for)
    template <size_t Index>
    inline typename std::enable_if<Index < sizeof...(Iters), void>::type IncrementIndex() {
        ++std::get<Index>(*this);
        IncrementIndex<Index+1>();
    }

    template <size_t Index>
    inline typename std::enable_if<Index == sizeof...(Iters), void>::type IncrementIndex() {}

    // TODO: add comments about all dark magic used here
    template <size_t... Indexes>
    inline typename std::enable_if<sizeof...(Indexes) != 0, bool>::type AnyEquals(const ZipIterator& other, std::integer_sequence<size_t, Indexes...>) const {
        return (... || (std::get<Indexes>(*this) == std::get<Indexes>(other)));
    }

    // TODO: consider if it is possible to safely remove this function and enable_if from the return value of the previous one.
    template <size_t... Indexes>
    inline typename std::enable_if<sizeof...(Indexes) == 0, bool>::type AnyEquals(const ZipIterator&, std::integer_sequence<size_t, Indexes...>) const {
        return true;
    }
};

template<typename... Types>
ZipIterator<Types...>& ZipIterator<Types...>::operator++() {
    IncrementIndex<0>();
    return *this;
}

template<typename... Types>
typename ZipIterator<Types...>::value_type ZipIterator<Types...>::operator*() {
    return ZipIterator::value_type();
}

template<typename... Types>
bool ZipIterator<Types...>::operator==(const ZipIterator& other) const {
    if (this == &other)
        return true;
    // TODO: write some comments about why AnyEquals is used here, not AllEquals or something.
    return AnyEquals(other, std::index_sequence_for<Types...>{});
}

template <typename... Types>
class Zip {
public:
    explicit Zip(Types&& ... args);
};

template<typename... Types>
Zip<Types...> zip(Types&& ... args) {
    return Zip<Types...>(std::forward<Types>(args)...);
}

template<typename... Types>
Zip<Types...>::Zip(Types&& ... args) {
    // TODO: store iterators
}
