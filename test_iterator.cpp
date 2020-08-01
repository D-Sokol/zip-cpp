#include <string>
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
