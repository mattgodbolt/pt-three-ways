#pragma once

#include "Vec3.h"

class OrthoNormalBasis {
  Vec3 x_;
  Vec3 y_;
  Vec3 z_;

public:
  OrthoNormalBasis(const Vec3 &x, const Vec3 &y, const Vec3 &z)
      : x_(x), y_(y), z_(z) {}

  constexpr const Vec3 &x() const noexcept { return x_; }
  constexpr const Vec3 &y() const noexcept { return y_; }
  constexpr const Vec3 &z() const noexcept { return z_; }

  constexpr const Vec3 transform(const Vec3 &pos) const noexcept {
    return x_ * pos.x() + y_ * pos.y() + z_ * pos.z();
  }

  static OrthoNormalBasis fromXY(const Vec3 &x, const Vec3 &y);
  static OrthoNormalBasis fromYX(const Vec3 &y, const Vec3 &x);

  static OrthoNormalBasis fromXZ(const Vec3 &x, const Vec3 &z);
  static OrthoNormalBasis fromZX(const Vec3 &z, const Vec3 &x);

  static OrthoNormalBasis fromYZ(const Vec3 &y, const Vec3 &z);
  static OrthoNormalBasis fromZY(const Vec3 &z, const Vec3 &y);

  static OrthoNormalBasis fromZ(const Vec3 &z);
};
