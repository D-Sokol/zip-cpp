#pragma once
#include <tuple>
#include <utility>

template <typename... Iters>
class ZipIterator : private std::tuple<Iters...> {
    using Base = std::tuple<Iters...>;
public:
    using Base::Base;
    // Something like tuple<int&, int&>. Used as return value of non-constant version of operator*.
    using value_type = std::tuple<typename std::iterator_traits<Iters>::reference...>;
    // Something like tuple<const int&, const int&>. Used as return value of constant version of operator*.
    using const_value_type = std::tuple<typename std::add_lvalue_reference_t<const typename std::iterator_traits<Iters>::value_type>...>;

    ZipIterator& operator++();
    value_type operator*();
    const_value_type operator*() const;

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

    template <size_t... Indexes>
    inline typename std::enable_if<sizeof...(Indexes) != 0, bool>::type AnyEquals(const ZipIterator& other, std::integer_sequence<size_t, Indexes...>) const {
        return (... || (std::get<Indexes>(*this) == std::get<Indexes>(other)));
    }

    // Так как вызов функции zip() без аргументов должен возвращать пустой диапазон,
    //  то в случае, когда список типов Types пуст, все итераторы должны считаться равными между собой.
    // Поскольку при использовании fold expression для оператора || на пустом списке
    //  возвращает false (https://en.cppreference.com/w/cpp/language/fold), необходима отдельная функция.
    // TODO: возможно ли использовать одну функцию и constexpr?
    template <size_t... Indexes>
    inline typename std::enable_if<sizeof...(Indexes) == 0, bool>::type AnyEquals(const ZipIterator&, std::integer_sequence<size_t, Indexes...>) const {
        return true;
    }

    template <size_t... Indexes>
    inline value_type CombineValues(std::integer_sequence<size_t, Indexes...>) {
        return std::forward_as_tuple(*std::get<Indexes>(*this)...);
    }

    template <size_t... Indexes>
    inline const_value_type CombineValues(std::integer_sequence<size_t, Indexes...>) const {
        return std::forward_as_tuple(*std::get<Indexes>(*this)...);
    }
};

template<typename... Types>
ZipIterator<Types...>& ZipIterator<Types...>::operator++() {
    IncrementIndex<0>();
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
