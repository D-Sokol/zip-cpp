cmake_minimum_required(VERSION 3.16)
project(zip)

set(CMAKE_CXX_STANDARD 17)

set(ZIP_SOURCES
    zip.h
)

set(TEST_SOURCES
    main.cpp test_iterator.cpp test_const_iterator.cpp test_zip.cpp test_iteration.cpp
)

add_library(zip ${ZIP_SOURCES})
set_target_properties(zip PROPERTIES LINKER_LANGUAGE CXX)

add_executable(test ${TEST_SOURCES})

target_link_libraries(test zip gtest gtest_main)

target_include_directories(test PRIVATE
        "extern/googletest/googletest/include")

add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")
