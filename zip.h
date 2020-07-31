#pragma once
#include <tuple>

template <typename... Iters>
class ZipIterator : private std::tuple<Iters...> {
    using Base = std::tuple<Iters...>;
public:
    using Base::Base;
    using value_type = std::tuple<typename std::iterator_traits<Iters>::value_type...>;

    ZipIterator& operator++();
    value_type operator*();

    bool operator==(const ZipIterator& other) const;

    inline const Base& AsTuple() const { return *this; }
private:
    template <size_t Index>
    inline typename std::enable_if<Index < sizeof...(Iters), void>::type IncrementIndex() {
        ++std::get<Index>(*this);
        IncrementIndex<Index+1>();
    }

    template <size_t Index>
    inline typename std::enable_if<Index == sizeof...(Iters), void>::type IncrementIndex() {}
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
    return false;
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
