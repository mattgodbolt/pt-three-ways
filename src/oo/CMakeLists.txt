add_library(oo Material.h Primitive.cpp Primitive.h Scene.cpp Scene.h ObjLoader.cpp ObjLoader.h Triangle.cpp Triangle.h Sphere.cpp Sphere.h)
target_link_libraries(oo math CONAN_PKG::CTRE)
target_include_directories(oo INTERFACE ..)