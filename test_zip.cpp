#include <vector>
#include <set>
#include "gtest/gtest.h"
#include "zip.h"

using namespace std;
using namespace zip_impl;

TEST(Construct, VectorsInitialization) {
    vector<int> v1 = {1, 2, 3};
    vector<char> v2 = {'a', 'b', 'c'};
    auto z = zip(v1, v2);
}

TEST(Construct, ConstVectorsInitialization) {
    const vector<int> v1 = {1, 2, 3};
    const vector<char> v2 = {'a', 'b', 'c'};
    auto z = zip(v1, v2);
}

TEST(Construct, DefaultConstructable) {
    auto z = zip();
}

TEST(Zip, BeginMethod) {
    {
        vector<int> a = {10, 20, 30};
        const vector<int> b = {40, 50, 60};
        auto z = zip(a, b);
        static_assert(is_same_v<typename decltype(z)::iterator, ZipIterator<vector<int>::iterator, vector<int>::const_iterator>>);
        auto [a_it, b_it] = z.begin().AsTuple();
        ASSERT_EQ(a_it, a.begin());
        ASSERT_EQ(b_it, b.begin());
    }
    {
        int a[10] = {0};
        auto z = zip(a);
        static_assert(is_same_v<typename decltype(z)::iterator, ZipIterator<int*>>);
        auto [a_it] = z.begin().AsTuple();
        ASSERT_EQ(a_it, static_cast<int*>(a));
    }
    {
        set<int> s = {1, 2};
        const char c[16] = {'h'};
        auto z = zip(s, c);
        static_assert(is_same_v<typename decltype(z)::iterator, ZipIterator<set<int>::iterator, const char*>>);
        auto [s_it, c_it] = z.begin().AsTuple();
        ASSERT_EQ(s_it, s.begin());
        ASSERT_EQ(c_it, static_cast<const char*>(c));
    }
    {
        static_assert(is_same_v<typename Zip<>::iterator, ZipIterator<>>);
    }
}

TEST(Zip, EndMethod) {
    {
        vector<int> a = {10, 20, 30};
        const vector<int> b = {40, 50, 60};
        auto z = zip(a, b);
        auto [a_it, b_it] = z.end().AsTuple();
        ASSERT_EQ(a_it, a.end());
        ASSERT_EQ(b_it, b.end());
    }
    {
        int a[10] = {0};
        auto z = zip(a);
        auto [a_it] = z.end().AsTuple();
        ASSERT_EQ(a_it, end(a));
    }
    {
        set<int> s = {1, 2};
        const char c[16] = {'h'};
        auto z = zip(s, c);
        auto [s_it, c_it] = z.end().AsTuple();
        ASSERT_EQ(s_it, s.end());
        ASSERT_EQ(c_it, end(c));
    }
}
