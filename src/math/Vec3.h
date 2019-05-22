#pragma once

#include "CeSupt.h"
#include <cmath>
#include <iosfwd>

class Vec3 {
  double x_{}, y_{}, z_{};

public:
  constexpr Vec3() noexcept = default;
  constexpr Vec3(double x, double y, double z) noexcept : x_(x), y_(y), z_(z) {}

  constexpr Vec3 operator+(const Vec3 &b) const noexcept {
    return Vec3(x_ + b.x_, y_ + b.y_, z_ + b.z_);
  }
  constexpr Vec3 &operator+=(const Vec3 &b) noexcept {
    x_ += b.x_;
    y_ += b.y_;
    z_ += b.z_;
    return *this;
  }

  constexpr Vec3 operator-(const Vec3 &b) const noexcept {
    return Vec3(x_ - b.x_, y_ - b.y_, z_ - b.z_);
  }
  constexpr Vec3 &operator-=(const Vec3 &b) noexcept {
    x_ -= b.x_;
    y_ -= b.y_;
    z_ -= b.z_;
    return *this;
  }

  constexpr Vec3 operator*(double b) const noexcept {
    return Vec3(x_ * b, y_ * b, z_ * b);
  }
  constexpr Vec3 &operator*=(double b) noexcept {
    x_ *= b;
    y_ *= b;
    z_ *= b;
    return *this;
  }

  constexpr Vec3 operator*(const Vec3 &b) const noexcept {
    return Vec3(x_ * b.x_, y_ * b.y_, z_ * b.z_);
  }
  constexpr Vec3 &operator*=(const Vec3 &b) noexcept {
    x_ *= b.x_;
    y_ *= b.y_;
    z_ *= b.z_;
    return *this;
  }

  constexpr Vec3 operator-() const noexcept { return Vec3(-x_, -y_, -z_); }

  constexpr double lengthSquared() const noexcept { return dot(*this); }
  constexpr double length() const noexcept {
    return ce_supt::sqrt(lengthSquared());
  }

  constexpr Vec3 normalised() const noexcept {
    return *this * (1.0 / length());
  }
  constexpr Vec3 &normalise() noexcept {
    *this = normalised();
    return *this;
  }

  constexpr double dot(const Vec3 &b) const noexcept {
    return x_ * b.x_ + y_ * b.y_ + z_ * b.z_;
  }

  constexpr Vec3 cross(const Vec3 &b) const noexcept {
    auto x = y_ * b.z_ - z_ * b.y_;
    auto y = z_ * b.x_ - x_ * b.z_;
    auto z = x_ * b.y_ - y_ * b.x_;
    return Vec3(x, y, z);
  }

  constexpr bool operator==(const Vec3 &b) const noexcept {
    return x_ == b.x_ && y_ == b.y_ && z_ == b.z_;
  }
  constexpr bool operator!=(const Vec3 &b) const noexcept {
    return x_ != b.x_ || y_ != b.y_ || z_ != b.z_;
  }

  constexpr double x() const noexcept { return x_; }
  constexpr double y() const noexcept { return y_; }
  constexpr double z() const noexcept { return z_; }
};

std::ostream &operator<<(std::ostream &o, const Vec3 &v);