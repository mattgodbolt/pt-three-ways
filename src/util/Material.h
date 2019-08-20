#pragma once

#include "math/Vec3.h"

#include <cmath>
#include <tuple>

struct Material {
  Vec3 emission;
  Vec3 diffuse;
  double reflectivity{};
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
    return std::tie(emission, diffuse, reflectivity, gloss)
           == std::tie(rhs.emission, rhs.diffuse, rhs.reflectivity, gloss);
  }
  bool operator!=(const Material &rhs) const { return !(rhs == *this); }
  constexpr double reflectionConeAngle() const { return (1 - gloss) * M_PI; }
};
