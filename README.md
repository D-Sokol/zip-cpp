# Zip-cpp

`Zip-cpp` - это написанная на C++ библиотека, представляющая функцию шаблонную функцию `zip`, имитирующую поведение одноименной функции в Python 3.
Эта функция возвращает объект, поддерживающий семантику range-based for и позволяющий параллельно итерироваться по нескольким контейнерам.

## Использование zip в python

Вызов zip используется для прохождения по произвольному количеству итерируемых объектов произвольных типов.
Например, следующий код:
```python3
iterable1 = [1, 2, 3, 4]
iterable2 = "abcde"  # Этот контейнер длиннее остальных
iterable3 = {40, 30, 20, 10}
for v1, v2, v3 in zip(iterable1, iterable2, iterable3):
    print(v1, v2, v3)
```
генерирует приведенный ниже вывод с точностью до порядка элементов в множестве:
```
1 a 40
2 b 10
3 c 20
4 d 30
```

При этом цикл завершается, как только итерация достигает конца хотя бы одного из диапазонов.

## Использование zip в C++

При помощи данной библиотеки приведенный выше код на Python можно переписать на C++ с сохранением семантики:
```c++
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include "zip.h"

int main(int, char**) {
    std::vector<int> iterable1 = {1, 2, 3, 4};
    std::string iterable2 = "abcde";
    std::unordered_set<int> iterable3 = {40, 30, 20, 10};
    for (const auto& [v1, v2, v3] : zip(iterable1, iterable2, iterable3)) {
        std::cout << v1 << ' '
                  << v2 << ' '
                  << v3 << std::endl;
    }
    return 0;
}
```


