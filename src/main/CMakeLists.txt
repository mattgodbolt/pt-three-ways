add_executable(pt_three_ways main.cpp PngWriter.cpp PngWriter.h)
target_link_libraries(pt_three_ways math oo util ${CMAKE_THREAD_LIBS_INIT} CONAN_PKG::clara CONAN_PKG::libpng)
