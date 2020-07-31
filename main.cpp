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

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
