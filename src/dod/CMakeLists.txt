add_library(dod Sphere.h TriangleVertices.cpp TriangleVertices.h IntersectionRecord.h Scene.cpp Scene.h)
target_link_libraries(dod math)
target_include_directories(dod INTERFACE ..)