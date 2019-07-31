add_library(oo Primitive.cpp Primitive.h Scene.cpp Scene.h Triangle.cpp Triangle.h Sphere.cpp Sphere.h)
target_link_libraries(oo math)
target_include_directories(oo INTERFACE ..)