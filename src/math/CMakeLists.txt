add_library(math Vec3.cpp Vec3.h Sphere.cpp Sphere.h Ray.cpp Ray.h Hit.cpp Hit.h CeSupt.h)
target_include_directories(math INTERFACE ..)