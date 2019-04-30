add_executable(math_tests math_tests.cpp Vec3Test.cpp math_tests.cpp)
target_link_libraries(math_tests math CONAN_PKG::Catch2)
add_test(NAME "Math tests" COMMAND math_tests)