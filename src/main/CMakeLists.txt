add_executable(pt_three_ways main.cpp Progressifier.cpp Progressifier.h WorkQueue.h)
target_link_libraries(pt_three_ways math oo ${CMAKE_THREAD_LIBS_INIT} CONAN_PKG::clara)
