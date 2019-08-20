#include "Sphere.h"

using oo::Sphere;

bool Sphere::intersect(const Ray &ray, Hit &hit) const noexcept {
  // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
  auto op = centre_ - ray.origin();
  auto radiusSquared = radius_ * radius_;
  auto b = op.dot(ray.direction());
  auto determinant = b * b - op.lengthSquared() + radiusSquared;
  if (determinant < 0)
    return false;

  determinant = sqrt(determinant);
  static constexpr auto epsilon = 1e-4;
  auto minusT = b - determinant;
  auto plusT = b + determinant;
  if (minusT < epsilon && plusT < epsilon)
    return false;

  auto t = minusT > epsilon ? minusT : plusT;
  auto hitPosition = ray.positionAlong(t);
  auto normal = (hitPosition - centre_).normalised();
  if (normal.dot(ray.direction()) > 0)
    normal = -normal;
  hit = Hit{t, hitPosition, normal};
  return true;
}
