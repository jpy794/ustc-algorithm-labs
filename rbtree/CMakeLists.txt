cmake_minimum_required(VERSION 3.24)
project(interval_tree)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

add_executable(test test.cpp)
add_executable(data_gen data_gen.cpp)
add_executable(main main.cpp)