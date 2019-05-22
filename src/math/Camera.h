#pragma once

#include "OrthoNormalBasis.h"
#include "Ray.h"
#include "Vec3.h"

class Camera {
  OrthoNormalBasis axis_;
  double lensRadius_;
  double u0_, u1_, v0_, v1_;
  double d_;
  Vec3 centre_, corner_, across_, up_;

public:
  Camera(const Vec3 &eye, const Vec3 &dir, const Vec3 &up, double aperture,
         double left, double right, double top, double bottom, double distance)
      : axis_(OrthoNormalBasis::fromZY(dir, up)), lensRadius_(aperture / 2),
        u0_(left), u1_(right), v0_(top), v1_(bottom), d_(distance),
        centre_(eye), corner_(centre_ + axis_.transform(Vec3(u0_, v0_, d_))),
        across_(axis_.x() * (u1_ - u0_)), up_(axis_.y() * (v1_ - v0_)) {}

  Ray ray(double x, double y, double xi1, double xi2) const {
    auto origin = centre_ + axis_.transform(
                                Vec3(2 * ((xi1 * xi1) - 0.5) * lensRadius_,
                                     2 * ((xi2 * xi2) - 0.5) * lensRadius_, 0));
    auto target = corner_ + across_ * x + up_ * y;
    return Ray::fromTwoPoints(origin, target);
  }
};