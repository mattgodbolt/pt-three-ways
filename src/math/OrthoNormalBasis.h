#pragma once

#include "Vec3.h"

class OrthoNormalBasis {
  Norm3 x_;
  Norm3 y_;
  Norm3 z_;

public:
  OrthoNormalBasis(const Norm3 &x, const Norm3 &y, const Norm3 &z)
      : x_(x), y_(y), z_(z) {}

  [[nodiscard]] constexpr const Norm3 &x() const noexcept { return x_; }
  [[nodiscard]] constexpr const Norm3 &y() const noexcept { return y_; }
  [[nodiscard]] constexpr const Norm3 &z() const noexcept { return z_; }

  [[nodiscard]] constexpr Vec3 transform(const Vec3 &pos) const noexcept {
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
