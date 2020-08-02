#include <vector>
#include <map>
#include <sstream>
#include "gtest/gtest.h"
#include "zip.h"

using namespace std;

TEST(SimpleIteration, PrintVectors) {
    vector<int> a = {10, 20, 30, 40, 50};
    vector<int> b = {-1, -2, -3, -4, -5};

    int iteration = 1;
    for (auto [x, y] : zip(a, b)) {
        ASSERT_EQ(x, 10 * iteration);
        ASSERT_EQ(y, -iteration);
        ++iteration;
    }
}

TEST(SimpleIteration, CopyVector) {
    vector<int> a = {6, 6, 6, 6, 6};
    const vector<int> b = {3, 1, 4, 1, 5};
    // One must use 'const auto&' here instead of 'auto&'.
    // Despite that, x is non-constant reference to the i-th element of vector.
    // See https://en.cppreference.com/w/cpp/language/structured_binding , case 2.
    for (const auto& [x, y] : zip(a, b)) {
        ASSERT_EQ(x, 6);
        x = y;
    }
    ASSERT_EQ(a, b);
}

TEST(SimpleIteration, CopyVectorAsTuple) {
    vector<int> a = {6, 6, 6, 6, 6};
    const vector<int> b = {3, 1, 4, 1, 5};
    for (const auto& tup : zip(a, b)) {
        ASSERT_EQ(get<0>(tup), 6);
        get<0>(tup) = get<1>(tup);
    }
    ASSERT_EQ(a, b);
}

TEST(SimpleIteration, RawArray) {
    double a[10];
    int iterations_passed = 0;
    for (const auto& [x] : zip(a)) {
        x = iterations_passed + 0.5;
        ++iterations_passed;
    }
    ASSERT_EQ(iterations_passed, 10) << "Wrong number of iterations for C-style array";
    for (size_t i = 0; i < 10; ++i)
        ASSERT_EQ(a[i], i + 0.5);
}

TEST(SimpleIteration, EmptyStructures) {
    {
        vector<int> a, b;
        for (auto [x, y] : zip(a, b))
            ASSERT_TRUE(false) << "Call zip with empty ranges should produce an empty range";
    }
    {
        for (auto _ : zip())
            ASSERT_TRUE(false) << "Call zip without arguments should produce an empty range";
    }
}

TEST(Iteration, DifferentLengths) {
    struct TestCase {
        const string s;
        vector<int> v;
        char c[5];
        size_t min_length() const {
            return min(s.size(), min(v.size(), sizeof(c)));
        }
    };

    vector<TestCase> cases = {
        TestCase{"abc", {10, 20, 30, 40, 50, 60}, "1234"},
        TestCase{"abcdefgh", {10, 20}, "1234"},
        TestCase{"", {10, 20, 30, 40, 50, 60}, "1234"},
        TestCase{"abcdefgh", {10, 20, 30, 40, 50, 60}, "1234"}
    };

    for (const auto& test_case : cases) {
        size_t iterations_passed = 0;
        for (const auto& [s_el, v_el, c_el] : zip(test_case.s, test_case.v, test_case.c)) {
            EXPECT_EQ(s_el, static_cast<char>('a' + iterations_passed));
            EXPECT_EQ(v_el, 10 + 10 * iterations_passed);
            if (iterations_passed == 4)
                EXPECT_EQ(c_el, '\0');
            else
                EXPECT_EQ(c_el, static_cast<char>('1' + iterations_passed));
            ++iterations_passed;
        }
        ASSERT_EQ(iterations_passed, test_case.min_length()) << "Wrong number of iterations";
    }
}

TEST(Iteration, CompareMaps) {
    const map<char, int> first = {{'a', 1}, {'b', 2}, {'c', 3}, {'d', 4}};
    const map<char, int> second = {{'a', 1}, {'b', 4}, {'c', 9}, {'d', 16}};
    for (const auto& [pair1, pair2] : zip(first, second)) {
        const auto& [k1, v1] = pair1;
        const auto& [k2, v2] = pair2;
        ASSERT_EQ(k1, k2);
        ASSERT_EQ(v1 * v1, v2);
    }
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

class Range {
    int start_;
    int stop_;
public:
    Range(int start, int stop) : start_(start), stop_(stop) {}
    struct iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = int;
        using difference_type = int;
        using pointer = int*;
        using reference = int&;

        int element;
        int operator*() const { return element; }
        iterator operator++() { ++element; return *this; }
        bool operator==(const iterator& other) const { return element == other.element; }
        bool operator!=(const iterator& other) const { return !operator==(other); }
    };

    iterator begin() const { return {start_}; }
    iterator end() const { return {stop_}; }
};


TEST(Iteration, CustomTypes) {
    stringstream input_stream("one two three four five");
    Range count(1, 1000);
    IterRange input(istream_iterator<string>(input_stream), istream_iterator<string>{});

    map<string, int> expected = {
        {"one", 1},
        {"two", 2},
        {"three", 3},
        {"four", 4},
        {"five", 5}
    };

    map<string, int> obtained;

    for (const auto& [word, number] : zip(input, count)) {
        obtained[word] = number;
    }
}
