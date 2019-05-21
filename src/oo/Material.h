#pragma once

#include "math/Vec3.h"

struct Material {
  Vec3 emission;
  Vec3 diffuse;
  static Material makeDiffuse(const Vec3 &colour) {
    return Material{Vec3(), colour};
  }
};
