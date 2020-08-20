#include <vector>
#include <sstream>
#include "gtest/gtest.h"
#include "zip.h"

using namespace std;
using namespace zipcpp;

/* Функция zip может принимать итераторы различных типов.
 * В этом примере в zip передается пара итераторов потока ввода (используется обертка zipcpp::IterRange)
 *  и контейнер типа Range, определенного пользователем.
 */

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
        int& operator*() { return element; }
        const int& operator*() const { return element; }
        iterator operator++() { ++element; return *this; }
        bool operator==(const iterator& other) const { return element == other.element; }
        bool operator!=(const iterator& other) const { return !operator==(other); }
    };

    iterator begin() const { return {start_}; }
    iterator end() const { return {stop_}; }
};


TEST(AdvancedIteration, CustomTypes) {
    stringstream input_stream("one two three four five");
    Range count(1, 1000);  // Контейнер из целых чисел от 1 до 999 включительно.
    IterRange<istream_iterator<string>> input(istream_iterator<string>(input_stream), istream_iterator<string>{});

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
    ASSERT_EQ(expected, obtained);
}

/* В этом примере демонстрируется возможность использования объекта Zip для сортировки двух диапазонов одновременно.
 * Порядок элементов после сортировки будет таким же, как если бы все контейнеры были объединены в контейнер из кортежей, который затем был бы отсортирован.
 * При этом используется приведенная ниже реализация сортировки вставками, а не std::sort,
 *  поскольку std::sort при некоторых условиях использует перемещение элементов, а не вызов swap. Эта операция не поддерживается для ссылок.
 */

template <typename RandomIt>
void SortWithSwap(RandomIt first, RandomIt last) {
    if (first != last) {
        SortWithSwap(first, last - 1);
        while (last != first) {
            auto next = last - 1;
            if (*next < *last)
                return;
            //std::iter_swap(last, next);
            swap(*last, *next);
            last = next;
        }
    }
}

TEST(AdvancedIteration, SortWithZip) {
    vector<int> v1 = { 2,  4,  1,  3,  1,  1,  3,  4};
    vector<int> v2 = {22, 54, 41, 13, 11, 61, 43, 34};
    auto z = zip(v1, v2);
    SortWithSwap(z.begin(), z.end());

    const vector<int> expected1 = { 1,  1,  1,  2,  3,  3,  4,  4};
    const vector<int> expected2 = {11, 41, 61, 22, 13, 43, 34, 54};

    ASSERT_EQ(v1, expected1);
    ASSERT_EQ(v2, expected2);
}
