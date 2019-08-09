#pragma once

#include "Scene.h"
#include "math/Camera.h"
#include "util/ArrayOutput.h"
#include <functional>

namespace fp {

void render(const Camera &camera, const Scene &scene, ArrayOutput &output,
            int samplesPerPixel, bool preview,
            const std::function<void()> &updateFunc);

}