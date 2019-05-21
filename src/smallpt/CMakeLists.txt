add_executable(small_pt smallpt.cpp)
target_compile_options(small_pt PUBLIC -fopenmp)
target_link_options(small_pt PUBLIC -fopenmp)