#pragma once

#include "math/Vec3.h"

#include <tuple>

struct Material {
  Vec3 emission;
  Vec3 diffuse;
  double reflectivity{};
  static Material makeDiffuse(const Vec3 &colour) {
    return Material{Vec3(), colour, 0};
  }
  static Material makeLight(const Vec3 &colour) {
    return Material{colour, Vec3(), 0};
  }
  static Material makeReflective(const Vec3 &colour, double reflectivity) {
    return Material{Vec3(), colour, reflectivity};
  }
  bool operator==(const Material &rhs) const {
    return std::tie(emission, diffuse, reflectivity)
           == std::tie(rhs.emission, rhs.diffuse, rhs.reflectivity);
  }
  bool operator!=(const Material &rhs) const { return !(rhs == *this); }
};
