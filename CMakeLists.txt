cmake_minimum_required(VERSION 3.12)
project(pt_three_ways)

find_program(CONAN conan)
if (NOT CONAN)
    message(FATAL_ERROR "Needs conan package manager: please visit https://conan.io and install")
endif ()

include(${CMAKE_SOURCE_DIR}/scripts/conan.cmake)

conan_check(VERSION 1.14.4 REQUIRED)
conan_add_remote(NAME bincrafters INDEX 1
        URL https://api.bintray.com/conan/bincrafters/public-conan)
conan_cmake_run(CONANFILE conanfile.txt
        BASIC_SETUP CMAKE_TARGETS
        BUILD missing)

set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_compile_options(-Wall -Werror -Wextra -march=native -g -funsafe-math-optimizations)
find_package(Threads)

add_subdirectory(src)
add_subdirectory(test)

if (NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTOAvailable)
endif ()

if (LTOAvailable)
    message("Link-time optimization enabled")

    set_property(TARGET oo PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET fp PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET util PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET math PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET pt_three_ways PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()