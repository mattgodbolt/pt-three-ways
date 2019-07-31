add_library(fp FpSpike.cpp Triangle.h Triangle.cpp Sphere.cpp Sphere.h)
target_link_libraries(fp math)
target_include_directories(fp INTERFACE ..)