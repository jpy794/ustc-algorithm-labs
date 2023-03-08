cmake_minimum_required(VERSION 3.24)
project(johnson)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)

add_executable(data_gen data_gen.cpp)
add_executable(main main.cpp)