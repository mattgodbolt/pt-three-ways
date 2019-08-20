#pragma once

#include "Scene.h"
#include "math/Camera.h"
#include "util/ArrayOutput.h"
#include <functional>
#include <util/RenderParams.h>

namespace fp {

void render(const Camera &camera, const Scene &scene,
            const RenderParams &renderParams, ArrayOutput &output,
            const std::function<void()> &updateFunc);

}