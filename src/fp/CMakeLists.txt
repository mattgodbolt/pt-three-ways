add_library(fp Render.cpp Triangle.h Triangle.cpp Sphere.cpp Sphere.h ../util/ObjLoaderImpl.h Scene.cpp Scene.h Primitive.h SceneBuilder.cpp SceneBuilder.h Render.h)
target_link_libraries(fp math)
target_include_directories(fp INTERFACE ..)