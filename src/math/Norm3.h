#pragma once

#include <cassert>

class Vec3;

class Norm3 {
  double x_{}, y_{}, z_{};

  friend class Vec3;
  constexpr explicit Norm3(const Vec3 &vec) noexcept;
  constexpr explicit Norm3(double x, double y, double z) noexcept
      : x_(x), y_(y), z_(z) {}

public:
  constexpr Norm3() noexcept = default;

  [[nodiscard]] constexpr Vec3 toVec3() const noexcept;

  static Norm3 fromNormal(const Vec3 &normal);

  constexpr bool operator==(const Norm3 &b) const noexcept {
    return x_ == b.x_ && y_ == b.y_ && z_ == b.z_;
  }
  constexpr bool operator!=(const Norm3 &b) const noexcept {
    return x_ != b.x_ || y_ != b.y_ || z_ != b.z_;
  }

  [[nodiscard]] constexpr Norm3 operator-() const noexcept {
    return Norm3(-x_, -y_, -z_);
  }

  [[nodiscard]] constexpr Norm3 reflect(const Norm3 &incoming) const noexcept;

  [[nodiscard]] constexpr double dot(const Vec3 &b) const noexcept;
  [[nodiscard]] constexpr double dot(const Norm3 &b) const noexcept;
  [[nodiscard]] constexpr Vec3 cross(const Vec3 &b) const noexcept;
  [[nodiscard]] constexpr Vec3 cross(const Norm3 &b) const noexcept;

  [[nodiscard]] constexpr Vec3 operator*(double b) const noexcept;

  [[nodiscard]] constexpr double x() const noexcept { return x_; }
  [[nodiscard]] constexpr double y() const noexcept { return y_; }
  [[nodiscard]] constexpr double z() const noexcept { return z_; }

  [[nodiscard]] static constexpr Norm3 xAxis() { return Norm3(1, 0, 0); }
  [[nodiscard]] static constexpr Norm3 yAxis() { return Norm3(0, 1, 0); }
  [[nodiscard]] static constexpr Norm3 zAxis() { return Norm3(0, 0, 1); }
};

#include "Vec3.h"

inline constexpr Norm3::Norm3(const Vec3 &vec) noexcept
    : x_(vec.x()), y_(vec.y()), z_(vec.z()) {}

inline constexpr Vec3 Norm3::toVec3() const noexcept {
  return Vec3(x_, y_, z_);
}

inline constexpr Vec3 Norm3::operator*(double b) const noexcept {
  return Vec3(x_ * b, y_ * b, z_ * b);
}

inline constexpr Vec3 Norm3::cross(const Norm3 &b) const noexcept {
  return cross(b.toVec3());
}

inline constexpr Vec3 Norm3::cross(const Vec3 &b) const noexcept {
  auto x = y_ * b.z() - z_ * b.y();
  auto y = z_ * b.x() - x_ * b.z();
  auto z = x_ * b.y() - y_ * b.x();
  return Vec3(x, y, z);
}

inline Norm3 Norm3::fromNormal(const Vec3 &normal) {
  assert(fabs(normal.lengthSquared() - 1.0) < 0.0000001); // TODO epsilon
  return Norm3(normal);
}

inline constexpr double Norm3::dot(const Vec3 &b) const noexcept {
  return x_ * b.x() + y_ * b.y() + z_ * b.z();
}

inline constexpr double Norm3::dot(const Norm3 &b) const noexcept {
  return x_ * b.x_ + y_ * b.y_ + z_ * b.z_;
}

inline constexpr Norm3 Norm3::reflect(const Norm3 &incoming) const noexcept {
  // We know by construction that the result of tihs operation is a normal.
  return Norm3(incoming.toVec3() - toVec3() * 2 * dot(incoming));
}
