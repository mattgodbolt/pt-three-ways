#pragma once

#include <cmath>

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

  constexpr Vec3 operator*(double b) const noexcept { return Vec3(x_ * b, y_ * b, z_ * b); }
  constexpr Vec3 &operator*=(double b) noexcept {
    x_ *= b;
    y_ *= b;
    z_ *= b;
    return *this;
  }

  constexpr Vec3 operator*(const Vec3 &b) const noexcept {
    return Vec3(x_ * b.x_, y_ * b.y_, z_ * b.z_);
  }

  constexpr Vec3 &norm() noexcept {
    *this *= (1 / sqrt(x_ * x_ + y_ * y_ + z_ * z_));
    return *this;
  }

  constexpr double dot(const Vec3 &b) const noexcept { return x_ * b.x_ + y_ * b.y_ + z_ * b.z_; }

  constexpr Vec3 cross(Vec3 &b) noexcept {
    return Vec3(y_ * b.z_ - z_ * b.y_, z_ * b.x_ - x_ * b.z_,
                x_ * b.y_ - y_ * b.x_);
  }

  constexpr double x() const noexcept { return x_; }
  constexpr double y() const noexcept { return y_; }
  constexpr double z() const noexcept { return z_; }
};
