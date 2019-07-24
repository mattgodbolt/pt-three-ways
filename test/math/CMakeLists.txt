add_executable(math_tests math_tests.cpp Vec3Tests.cpp RayTests.cpp SphereTests.cpp ApproxVec3.cpp ApproxVec3.h OrthoNormalBasisTests.cpp TriangleTests.cpp)
target_link_libraries(math_tests math CONAN_PKG::Catch2)
add_test(NAME math_tests COMMAND $<TARGET_FILE:math_tests>)