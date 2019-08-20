add_library(util Material.h ObjLoader.h ObjLoader.cpp ObjLoaderImpl.h SampledPixel.cpp SampledPixel.h ArrayOutput.cpp ArrayOutput.h WorkQueue.h
        Progressifier.cpp Progressifier.h RenderParams.cpp RenderParams.h)
target_link_libraries(util math CONAN_PKG::CTRE CONAN_PKG::date)
target_include_directories(util INTERFACE ..)