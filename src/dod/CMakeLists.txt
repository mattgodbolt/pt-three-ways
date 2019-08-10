add_library(dod DodSpike.cpp)
target_link_libraries(dod math)
target_include_directories(dod INTERFACE ..)