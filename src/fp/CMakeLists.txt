add_library(fp FpSpike.cpp)
target_link_libraries(fp math)
target_include_directories(fp INTERFACE ..)