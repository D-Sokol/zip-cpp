# Zip-cpp

`Zip-cpp` - это написанная на C++ библиотека, представляющая функцию шаблонную функцию `zip`, имитирующую поведение одноименной функции в Python 3.
Эта функция возвращает объект, поддерживающий семантику range-based for и позволяющий параллельно итерироваться по нескольким контейнерам.

## Возможности библиотеки

Функция zip поддерживает следующие контейнеры:
* Все контейнеры STL (vector, string, set, map, list, ...).
* Массивы в стиле C (int[10]), при условии, что тип соответствующего выражения не был преобразован в указатель.
* Любые диапазоны, заданные двумя итераторами, удовлетворяющими требованиям категории InputIterator и шаблона [iterator_traits](https://en.cppreference.com/w/cpp/iterator/iterator_traits), в том числе, определенными пользователем.
  Пару итераторов необходимо скомпоновать в единый объект, при помощи тривиального класса-обертки, предоставляющего методы begin, end.
  Пример использования этой возможности для [итераторов потока ввода](https://en.cppreference.com/w/cpp/iterator/istream_iterator) и числовой последовательности приведен в файле `test_iteration.cpp`.

Если все переданные на вход объекты предоставляют итераторы, относящиеся к категории ForwardIterator, то функция `zip` возвращает итератор этой же категории, в противном случае возвращается InputIterator.
Так как основным сценарием использования функции `zip` является использование в range-based for, поддержка остальных категорий итераторов является задачей низкого приоритета.  

## Пример использования

### Использование zip в python

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

### Использование zip в C++

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

## Использование библиотеки

Все классы и функции, необходимые для использования `zip`, находятся в заголовочном файле zip.h.
Остальные файлы с исходным кодом в данном репозитории предоставляют юнит-тесты для библиотеки, а также функцию main, вызывающую эти тесты.

Наиболее простым способом использования данной библиотеки в другом проекте является копирование файла zip.h.

В качестве альтернативного решения для проектов, использующих Git и CMake можно добавить данный репозиторий в качестве подмодуля и соответствующим образом настроить команды `add_subdirectory`, `target_include_directories`, `target_link_libraries`.

Для сборки проекта, использующего данную библиотеку, необходимо использовать версию стандарта не ниже C++17.
