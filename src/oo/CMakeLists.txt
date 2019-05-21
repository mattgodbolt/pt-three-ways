add_library(oo Material.h Primitive.cpp Primitive.h Scene.cpp Scene.h)
target_link_libraries(oo math)
target_include_directories(oo INTERFACE ..)