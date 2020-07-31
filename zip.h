#pragma once
#include <tuple>

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
