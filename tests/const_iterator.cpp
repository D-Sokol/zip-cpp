#include <deque>
#include <forward_list>
#include <iterator>
#include <numeric>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include "gtest/gtest.h"
#include "zip.h"

using namespace std;
using namespace zipcpp;
using namespace zip_impl;

template<typename T>
inline constexpr bool is_const_ref = is_const_v<remove_reference_t<T>>;

TEST(ConstIterator, Constructable) {
    vector<int> a = {10, 20};
    const int b[4] = {1, 2, 3, 4};
    string s = "abcdef";
    auto it = ConstZipIterator<vector<int>::iterator, const int*, string::iterator>(a.begin(), static_cast<const int*>(b), s.begin());
}

TEST(ConstIterator, DefaultConstructable) {
    auto it = ConstZipIterator<>();
}

TEST(ConstIterator, NestedZipConstructable) {
    vector<int> a = {10};
    vector<int> b = {10};
    vector<int> c = {10};
    vector<int> d = {10};
    const auto z1 = zip(a, b);
    const auto z2 = zip(c, d);

    for (const auto& [tup1, tup2] : zip(z1, z2)) {
        ASSERT_EQ(get<0>(tup1), 10);
        static_assert(is_const_ref<decltype(get<0>(tup1))>);
        ASSERT_EQ(get<1>(tup1), 10);
        static_assert(is_const_ref<decltype(get<1>(tup1))>);
        ASSERT_EQ(get<0>(tup2), 10);
        static_assert(is_const_ref<decltype(get<0>(tup2))>);
        ASSERT_EQ(get<1>(tup2), 10);
        static_assert(is_const_ref<decltype(get<1>(tup2))>);
    }
}


TEST(ConstIterator, NestedZipConstructableWithNonConst) {
    vector<int> a = {10};
    const vector<int> b = {10};
    vector<int> c = {10};
    const vector<int> d = {10};
    // Mixed constant and non-constant containers: only elements of a should be assignable.
    auto z1 = zip(a, b);
    const auto z2 = zip(c, d);

    for (const auto& [tup1, tup2] : zip(z1, z2)) {
        ASSERT_EQ(get<0>(tup1), 10);
        static_assert(! is_const_ref<decltype(get<0>(tup1))>);
        ASSERT_EQ(get<1>(tup1), 10);
        static_assert(is_const_ref<decltype(get<1>(tup1))>);
        ASSERT_EQ(get<0>(tup2), 10);
        static_assert(is_const_ref<decltype(get<0>(tup2))>);
        ASSERT_EQ(get<1>(tup2), 10);
        static_assert(is_const_ref<decltype(get<1>(tup2))>);
    }
}

TEST(ConstIterator, VectorIncrement) {
    vector<int> a = {10, 20};
    const vector<int> b = {30, 40, 50};
    auto it = ConstZipIterator<vector<int>::iterator, vector<int>::const_iterator>(a.begin(), b.begin() + 1);
    ++it;
    ASSERT_EQ(it.AsTuple(), make_tuple(a.begin() + 1, b.begin() + 2));
}

TEST(ConstIterator, EqualOperatorForReferenceable) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ConstZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;
    {
        ZI z1(a.begin(), s.begin(), c.begin());
        EXPECT_EQ(z1, z1) << "Iterator should be equal to itself";
    }
    {
        ZI z1(a.begin() + 2, s.begin() + 1, c.begin() + 1);
        EXPECT_EQ(z1, z1) << "Iterator should be equal to itself";
    }

    {
        ZI z1(a.begin() + 2, s.begin() + 1, c.begin() + 1);
        ZI z2(a.begin() + 2, s.begin() + 1, c.begin() + 1);
        EXPECT_EQ(z1, z2);
        EXPECT_EQ(z2, z1);
    }
    {
        ZI z1(a.begin() + 1, s.begin() + 1, c.begin() + 1);
        ZI z2(a.begin(), s.begin(), c.begin());
        EXPECT_NE(z1, z2);
    }
}

TEST(ConstIterator, EqualOperatorAfterIncrement) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ConstZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;

    ZI first(a.begin(), s.begin(), c.begin());
    ZI last(a.end(), s.end(), c.end());
    for (size_t first_increments = 0; first_increments < 2; ++first_increments, ++first) {
        ZI second(a.begin(), s.begin(), c.begin());
        for (size_t second_increments = 0; second_increments < 2; ++second_increments, ++second) {
            if (first_increments == second_increments)
                ASSERT_EQ(first, second) << "ConstZipIterators should be equal after " << first_increments << " increments";
            else
                ASSERT_NE(first, second) << "ConstZipIterators should not be equal after" << first_increments << " and " << second_increments << " increments";
        }
        EXPECT_NE(first, last) << "ConstZipIterator should not reach end after " << first_increments << " increments";
    }
    EXPECT_EQ(first, last) << "ConstZipIterator should reach end after 2 increments";
}

TEST(ConstIterator, EqualOperatorForEmptyZip) {
    ConstZipIterator<> zi1, zi2;
    ASSERT_EQ(zi1, zi2) << "All ConstZipIterators<> should be equal to each other";
}

TEST(ConstIterator, ReferenceReadValue) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ConstZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;

    {
        ZI z1(a.begin(), s.begin(), c.begin());
        // These assertions are different from the same in the test_iterator.cpp file (all references are const).
        static_assert(is_same_v<decltype(get<0>(*z1)), const int&>);
        static_assert(is_same_v<decltype(get<1>(*z1)), const char&>);
        static_assert(is_same_v<decltype(get<2>(*z1)), const bool&>);
        {
            auto[av, sv, cv] = *z1;
            ASSERT_EQ(av, 10);
            ASSERT_EQ(sv, 'a');
            ASSERT_EQ(cv, false);
        }
        {
            auto tup = *z1;
            ASSERT_EQ(tup, make_tuple(10, 'a', false));
        }
    }
    {
        ZI z1(a.begin() + 2, s.begin() + 1, c.begin() + 1);
        static_assert(is_same_v<decltype(get<0>(*z1)), const int&>);
        static_assert(is_same_v<decltype(get<1>(*z1)), const char&>);
        static_assert(is_same_v<decltype(get<2>(*z1)), const bool&>);
        auto[av, sv, cv] = *z1;
        ASSERT_EQ(av, 30);
        ASSERT_EQ(sv, 'b');
        ASSERT_EQ(cv, true);
    }
}

TEST(ConstIterator, ReferenceReadAfterIncrement) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ConstZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;
    {
        ZI z1(a.begin(), s.begin(), c.begin());
        ++z1;
        auto[av, sv, cv] = *z1;
        ASSERT_EQ(av, 20);
        static_assert(is_const_ref<decltype(av)>);
        ASSERT_EQ(sv, 'b');
        static_assert(is_const_ref<decltype(sv)>);
        ASSERT_EQ(cv, true);
        static_assert(is_const_ref<decltype(cv)>);
    }
}


template <typename Category, typename ... Iters>
using result_category = is_same<Category, typename ConstZipIterator<Iters...>::iterator_category>;

TEST(IteratorCategories, CheckMemberTypesConst) {
    using i_input = istream_iterator<int>;
    using i_forward = typename std::forward_list<int>::iterator;
    using i_bidir = typename std::set<int>::iterator;
    using i_random = typename std::deque<int>::iterator;

    static_assert(result_category<input_iterator_tag>::value);

    static_assert(result_category<input_iterator_tag, i_input>::value);
    static_assert(result_category<forward_iterator_tag, i_forward>::value);
    static_assert(result_category<bidirectional_iterator_tag, i_bidir>::value);
    static_assert(result_category<random_access_iterator_tag, i_random>::value);

    static_assert(result_category<input_iterator_tag, i_input, i_input, i_input>::value);
    static_assert(result_category<forward_iterator_tag, i_forward, i_forward, i_forward>::value);
    static_assert(result_category<bidirectional_iterator_tag, i_bidir, i_bidir, i_bidir>::value);
    static_assert(result_category<random_access_iterator_tag, i_random, i_random, i_random>::value);

    static_assert(result_category<input_iterator_tag, i_input, i_random, i_random>::value);
    static_assert(result_category<input_iterator_tag, i_random, i_input, i_random>::value);
    static_assert(result_category<input_iterator_tag, i_random, i_random, i_input>::value);

    static_assert(result_category<forward_iterator_tag, i_random, i_bidir, i_random, i_forward, i_bidir, i_forward, i_random>::value);

    static_assert(result_category<forward_iterator_tag, i_random, ConstZipIterator<i_bidir, i_random>, ConstZipIterator<i_forward, i_bidir>>::value);
    static_assert(result_category<forward_iterator_tag, i_random, ConstZipIterator<i_bidir, i_random>, ZipIterator<i_forward, i_bidir>>::value);
    static_assert(result_category<forward_iterator_tag, i_random, ZipIterator<i_bidir, i_random>, ConstZipIterator<i_forward, i_bidir>>::value);
    ASSERT_TRUE(true);
}

TEST(ConstIteratorCategories, BidirectionalDecrement) {
    std::set<int> a = {10, 40, 20};
    std::set<int> b = {1, 2, 4};
    const auto z = zip(a, b);
    auto it1 = z.begin();
    ++it1;
    EXPECT_EQ(*it1, make_tuple(20, 2));
    --it1;
    ASSERT_EQ(it1, z.begin());
    ASSERT_EQ(*it1, *z.begin());
}

TEST(ConstIteratorCategories, Arithmetics) {
    vector<int> a(10);
    iota(a.begin(), a.end(), 0);
    EXPECT_EQ(a.back(), 9);

    const auto z = zip(a);

    for (int i = 0; i < 10; ++i) {
        auto it1 = begin(z);
        auto it2 = it1 + i;
        auto it3 = i + it1;
        it1 += i;
        EXPECT_EQ(it1, it2);
        ASSERT_EQ(get<0>(*it1), i) << "Error in operator +=";
        ASSERT_EQ(get<0>(*it2), i) << "Error in operator +(iterator, int)";
        ASSERT_EQ(get<0>(*it3), i) << "Error in operator +(int, iterator)";
    }
    for (int i = 1; i < 11; ++i) {
        auto it1 = end(z);
        auto it2 = it1 - i;
        it1 -= i;
        EXPECT_EQ(it1, it2);
        ASSERT_EQ(get<0>(*it1), 10-i) << "Error in operator -=";
        ASSERT_EQ(get<0>(*it2), 10-i) << "Error in operator -";
    }
}

TEST(ConstIteratorCategories, Differences) {
    vector<int> a(10);
    const auto z = zip(a);
    for (int i = 0; i < 10; ++i) {
        auto it1 = z.begin() + i;
        for (int j = 0; j < 10; ++j) {
            auto it2 = z.begin() + j;
            ASSERT_EQ(it2 - it1, j - i);
        }
    }
}

TEST(ConstIteratorCategories, Comparisons) {
    vector<int> a(10);
    const auto z = zip(a);
    for (int i = 0; i < 10; ++i) {
        auto it1 = z.begin() + i;
        for (int j = 0; j < 10; ++j) {
            auto it2 = z.begin() + j;
            ASSERT_EQ(it2 >  it1, j >  i) << "Error in operator >";
            ASSERT_EQ(it2 >= it1, j >= i) << "Error in operator >=";
            ASSERT_EQ(it2 <  it1, j <  i) << "Error in operator <";
            ASSERT_EQ(it2 <= it1, j <= i) << "Error in operator <=";
        }
    }
}

TEST(ConstIteratorCategories, OperatosSquareBrackets) {
    vector<int> a(10);
    iota(a.begin(), a.end(), 0);
    const auto z = zip(a);
    for (int i = 0; i < 10; ++i) {
        auto it1 = z.begin() + i;
        for (int j = 0; j < 10 - i; ++j) {
            auto [value] = it1[j];
            ASSERT_EQ(value, i + j) << "(z.begin() + " << i << ")[" << j << "] returns wrong value";
        }
    }
}
