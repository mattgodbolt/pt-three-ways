cmake_minimum_required(VERSION 3.14)
project(pt_three_ways)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror -Wextra)

add_subdirectory(smallpt)

add_executable(pt_three_ways main.cpp)