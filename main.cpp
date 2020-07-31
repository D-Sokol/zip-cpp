#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "zip.h"

using namespace std;

TEST(Demo, FirstTest) {
    ASSERT_TRUE(true);
}

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


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
