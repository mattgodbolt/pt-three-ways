cmake_minimum_required(VERSION 3.12)
project(pt_three_ways)

find_program(CONAN conan)
if (NOT CONAN)
    message(FATAL_ERROR "Needs conan package manager: please visit https://conan.io and install")
endif ()

include(${CMAKE_SOURCE_DIR}/scripts/conan.cmake)

conan_add_remote(NAME Catch2 "https://api.bintray.com/conan/catchorg/Catch2")
conan_cmake_run(CONANFILE conanfile.txt
        BASIC_SETUP CMAKE_TARGETS
        BUILD missing)

set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_compile_options(-Wall -Werror -Wextra)

add_subdirectory(src)
add_subdirectory(test)

add_executable(pt_three_ways main.cpp)
target_link_libraries(pt_three_ways math)