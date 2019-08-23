#pragma once

#include "math/Vec3.h"

#include <cmath>

struct Material {
  Vec3 emission;
  Vec3 diffuse;
  double indexOfRefraction{1.0};
  double reflectivity{-1};
  double gloss{};
  static Material makeDiffuse(const Vec3 &colour) {
    return Material{Vec3(), colour, 0, 0};
  }
  static Material makeLight(const Vec3 &colour) {
    return Material{colour, Vec3(), 0, 0};
  }
  static Material makeReflective(const Vec3 &colour, double reflectivity,
                                 double gloss) {
    return Material{Vec3(), colour, reflectivity, gloss};
  }
  bool operator==(const Material &rhs) const {
    return emission == rhs.emission && diffuse == rhs.diffuse
           && indexOfRefraction == rhs.indexOfRefraction
           && reflectivity == rhs.reflectivity && gloss == rhs.gloss;
  }
  bool operator!=(const Material &rhs) const { return !(rhs == *this); }
  [[nodiscard]] constexpr double reflectionConeAngle() const {
    return (1 - gloss) * M_PI;
  }
};
