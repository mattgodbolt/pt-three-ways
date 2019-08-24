#pragma once

#include "math/Vec3.h"

#include <cmath>

struct Material {
  Vec3 emission;
  Vec3 diffuse;
  double indexOfRefraction{1.0};
  double reflectivity{-1};
  double reflectionConeAngleRadians{0.0};
  static double toRadians(double angle) { return angle / 360 * 2 * M_PI; }
  static Material makeDiffuse(const Vec3 &colour) {
    return Material{Vec3(), colour};
  }
  static Material makeSpecular(const Vec3 &colour, double index) {
    return Material{Vec3(), colour, index};
  }
  static Material makeLight(const Vec3 &colour) {
    return Material{colour, Vec3()};
  }
  static Material makeGlossy(const Vec3 &colour, double index,
                             double reflectionConeAngleDegrees) {
    return Material{Vec3(), colour, index, -1,
                    toRadians(reflectionConeAngleDegrees)};
  }
  static Material makeReflective(const Vec3 &colour, double reflectivity,
                                 double reflectionConeAngleDegrees) {
    return Material{Vec3(), colour, 1.0, reflectivity,
                    toRadians(reflectionConeAngleDegrees)};
  }
  bool operator==(const Material &rhs) const {
    return emission == rhs.emission && diffuse == rhs.diffuse
           && indexOfRefraction == rhs.indexOfRefraction
           && reflectivity == rhs.reflectivity
           && reflectionConeAngleRadians == rhs.reflectionConeAngleRadians;
  }
  bool operator!=(const Material &rhs) const { return !(rhs == *this); }
};
