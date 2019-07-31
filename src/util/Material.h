#pragma once

#include "math/Vec3.h"

struct Material {
  Vec3 emission;
  Vec3 diffuse;
  double reflectivity{};
  static Material makeDiffuse(const Vec3 &colour) {
    return Material{Vec3(), colour, 0};
  }
  static Material makeReflective(const Vec3 &colour, double reflectivity) {
    return Material{Vec3(), colour, reflectivity};
  }
};
