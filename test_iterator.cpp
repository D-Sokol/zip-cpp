#include <deque>
#include <forward_list>
#include <iterator>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include "gtest/gtest.h"
#include "zip.h"

using namespace std;

TEST(Iterator, Constructable) {
    vector<int> a = {10, 20};
    const int b[4] = {1, 2, 3, 4};
    string s = "abcdef";
    auto it = ZipIterator<vector<int>::iterator, const int*, string::iterator>(a.begin(), static_cast<const int*>(b), s.begin());
}

TEST(Iterator, DefaultConstructable) {
    auto it = ZipIterator<>();
}

TEST(Iterator, NestedZipConstructable) {
    vector<int> a = {10};
    vector<int> b = {10};
    vector<int> c = {10};
    vector<int> d = {10};
    auto z1 = zip(a, b);
    auto z2 = zip(c, d);

    using normal_zip_iter = remove_reference_t<decltype(z1.begin())>;
    using nested_zip_iter = remove_reference_t<decltype(zip(z1, z2).begin())>;

    static_assert(is_same_v<normal_zip_iter::value_type, tuple<int&, int&>>);
    static_assert( is_same_v<nested_zip_iter::value_type, tuple<tuple<int&, int&> , tuple<int&, int&> >>);
    static_assert(!is_same_v<nested_zip_iter::value_type, tuple<tuple<int&, int&>&, tuple<int&, int&>&>>);

    for (const auto& [tup1, tup2] : zip(z1, z2)) {
        ASSERT_EQ(get<0>(tup1), 10);
        ASSERT_EQ(get<1>(tup1), 10);
        ASSERT_EQ(get<0>(tup2), 10);
        ASSERT_EQ(get<1>(tup2), 10);
    }
}

TEST(Iterator, VectorIncrement) {
    vector<int> a = {10, 20};
    const vector<int> b = {30, 40, 50};
    auto it = ZipIterator<vector<int>::iterator, vector<int>::const_iterator>(a.begin(), b.begin() + 1);
    ++it;
    ASSERT_EQ(it.AsTuple(), make_tuple(a.begin() + 1, b.begin() + 2));
}

TEST(Iterator, EqualOperatorForReferenceable) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;
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

TEST(Iterator, EqualOperatorAfterIncrement) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;

    ZI first(a.begin(), s.begin(), c.begin());
    ZI last(a.end(), s.end(), c.end());
    for (size_t first_increments = 0; first_increments < 2; ++first_increments, ++first) {
        ZI second(a.begin(), s.begin(), c.begin());
        for (size_t second_increments = 0; second_increments < 2; ++second_increments, ++second) {
            if (first_increments == second_increments)
                ASSERT_EQ(first, second) << "ZipIterators should be equal after " << first_increments << " increments";
            else
                ASSERT_NE(first, second) << "ZipIterators should not be equal after" << first_increments << " and " << second_increments << " increments";
        }
        EXPECT_NE(first, last) << "ZipIterator should not reach end after " << first_increments << " increments";
    }
    EXPECT_EQ(first, last) << "ZipIterator should reach end after 2 increments";
}

TEST(Iterator, EqualOperatorForEmptyZip) {
    ZipIterator<> zi1, zi2;
    ASSERT_EQ(zi1, zi2) << "All ZipIterators<> should be equal to each other";
}

TEST(Iterator, ReferenceReadValue) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;

    {
        ZI z1(a.begin(), s.begin(), c.begin());
        static_assert(is_same_v<decltype(*z1), tuple<int&, char&, const bool&>>);
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
        static_assert(is_same_v<decltype(*z1), tuple<int&, char&, const bool&>>);
        auto[av, sv, cv] = *z1;
        ASSERT_EQ(av, 30);
        ASSERT_EQ(sv, 'b');
        ASSERT_EQ(cv, true);
    }
}

TEST(Iterator, ReferenceReadConstIterator) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;

    {
        const ZI z1(a.begin(), s.begin(), c.begin());
        static_assert(is_same_v<decltype(*z1), tuple<const int&, const char&, const bool&>>);
        auto[av, sv, cv] = *z1;
        ASSERT_EQ(av, 10);
        ASSERT_EQ(sv, 'a');
        ASSERT_EQ(cv, false);
    }
}

TEST(Iterator, ReferenceReadAfterIncrement) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;
    {
        ZI z1(a.begin(), s.begin(), c.begin());
        ++z1;
        auto[av, sv, cv] = *z1;
        ASSERT_EQ(av, 20);
        ASSERT_EQ(sv, 'b');
        ASSERT_EQ(cv, true);
    }
}

TEST(Iterator, Assignment) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;
    {
        ZI z1(a.begin(), s.begin(), c.begin());
        const auto& [av, sv, cv] = *z1; // FIXME: к чему здесь вообще относится const?
        av += 5;
        sv = 'A';
        ASSERT_EQ(av, 15);
        ASSERT_EQ(sv, 'A');
        ASSERT_EQ(a.front(), 15);
        ASSERT_EQ(s.front(), 'A');
    }
}


template <typename Category, typename ... Iters>
using result_category = is_same<Category, typename ZipIterator<Iters...>::iterator_category>;

TEST(IteratorCategories, CheckMemberTypes) {
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

    static_assert(result_category<forward_iterator_tag, i_random, ZipIterator<i_bidir, i_random>, ZipIterator<i_forward, i_bidir>>::value);
    ASSERT_TRUE(true);
}
