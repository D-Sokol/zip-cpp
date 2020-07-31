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
        ZI z1(a.begin() + 2, s.begin() + 1, c.begin() + 1);
        ZI z2(a.begin() + 2, s.begin() + 1, c.begin());
        EXPECT_NE(z1, z2);
        EXPECT_NE(z2, z1);
    }
    {
        ZI z1(a.begin() + 2, s.begin() + 1, c.begin() + 1);
        ZI z2(a.begin() + 1, s.begin() + 1, c.begin() + 1);
        EXPECT_NE(z1, z2);
        EXPECT_NE(z2, z1);
    }
    {
        ZI z1(a.begin() + 2, s.begin()    , c.begin() + 1);
        ZI z2(a.begin() + 2, s.begin() + 1, c.begin() + 1);
        EXPECT_NE(z1, z2);
        EXPECT_NE(z2, z1);
    }
    {
        ZI z1(a.begin() + 2, s.begin() + 1, c.begin() + 1);
        ZI z2(a.begin(), s.begin(), c.begin());
        EXPECT_NE(z1, z2);
    }
}

TEST(Iterator, EqualOperatorForNonReferenceable) {
    vector<int> a = {10, 20, 30};
    string s = "abcd";
    const array<bool, 2> c = {false, true};
    using ZI = ZipIterator<vector<int>::iterator, string::iterator, array<bool, 2>::const_iterator>;

    vector<ZI> iters;
    iters.reserve(8);
    for (auto ai : {a.begin(), a.end()})
        for (auto si : {s.begin(), s.end()})
            for (auto ci : {c.begin(), c.end()})
                iters.emplace_back(ai, si, ci);

    for (size_t i = 0; i < iters.size(); ++i) {
        for (size_t j = i; j < iters.size(); ++j) {
            if (i == 0) {
                if (j == 0) {
                    // In fact, this was already tested.
                    ASSERT_EQ(iters[i], iters[j]);
                } else {
                    EXPECT_NE(iters[i], iters[j]) << "Referenceable and nonreferenceable iterators should not be equal";
                }
            } else {
                EXPECT_EQ(iters[i], iters[j]) << "Iterators containing at least one end should be equal";
            }
        }
    }
}
