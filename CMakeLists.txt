cmake_minimum_required(VERSION 3.16)
project(zip)

set(CMAKE_CXX_STANDARD 17)

set(ZIP_SOURCES
    zip.h
)

set(TEST_SOURCES
    main.cpp
)

add_library(zip ${ZIP_SOURCES})
set_target_properties(zip PROPERTIES LINKER_LANGUAGE CXX)

add_executable(test main.cpp)

target_link_libraries(test zip)
