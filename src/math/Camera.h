#pragma once

#include "OrthoNormalBasis.h"
#include "Ray.h"
#include "Vec3.h"

#include <cmath>
#include <random>

class Camera {
  Vec3 centre_;
  OrthoNormalBasis axis_;
  double aspectRatio_;
  double cameraPlaneDist_;
  double apertureRadius_{};
  double focalDistance_{};

public:
  Camera(const Vec3 &eye, const Vec3 &lookAt, const Vec3 &up, int width,
         int height, double verticalFov)
      : centre_(eye),
        axis_(OrthoNormalBasis::fromZY((lookAt - eye).normalised(), up)),
        aspectRatio_(static_cast<double>(width) / height),
        cameraPlaneDist_(1.0 / tan(verticalFov * M_PI / 360.0)) {}

  void setFocus(const Vec3 &focalPoint, double apertureRadius) {
    focalDistance_ = (focalPoint - centre_).length();
    apertureRadius_ = apertureRadius;
  }

  template <typename Rng>
  [[nodiscard]] Ray ray(double x, double y, Rng &rng) const {
    auto xContrib = axis_.x() * -x * aspectRatio_;
    auto yContrib = axis_.y() * -y;
    auto zContrib = axis_.z() * cameraPlaneDist_;
    auto direction = (xContrib + yContrib + zContrib).normalised();
    if (apertureRadius_ == 0)
      return Ray::fromOriginAndDirection(centre_, direction);

    auto focalPoint = centre_ + direction * focalDistance_;
    std::uniform_real_distribution<> angleDist(0, 2 * M_PI);
    std::uniform_real_distribution<> radiusDist(0, apertureRadius_);
    auto angle = angleDist(rng);
    auto radius = radiusDist(rng);
    auto origin = centre_ + (axis_.x() * cos(angle) * radius)
                  + (axis_.y() * sin(angle) * radius);
    return Ray::fromTwoPoints(origin, focalPoint);
  }

  // TODO maybe use this more?
  template <typename Rng>
  [[nodiscard]] Ray ray(int x, int y, int width, int height, Rng &rng) const {
    std::uniform_real_distribution<> unit;
    auto u = unit(rng);
    auto v = unit(rng);
    auto yy = (2 * (static_cast<double>(y) + u + 0.5) / (height - 1)) - 1;
    auto xx = (2 * (static_cast<double>(x) + v + 0.5) / (width - 1)) - 1;
    return ray(xx, yy, rng);
  }
};