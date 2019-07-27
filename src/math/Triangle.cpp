#include "Triangle.h"

constexpr double Epsilon = 0.000000001;

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
std::optional<Hit> Triangle::intersect(const Ray &ray) const noexcept {
  auto pVec = ray.direction().cross(vVector());
  auto det = uVector().dot(pVec);
  // ray and triangle are parallel if det is close to 0
  if (fabs(det) < Epsilon)
    return {};

  auto backfacing = det < Epsilon;

  auto invDet = 1.0 / det;
  auto tVec = ray.origin() - vertices_[0];
  auto u = tVec.dot(pVec) * invDet;
  if (u < 0.0 || u > 1.0)
    return {};

  auto qVec = tVec.cross(uVector());
  auto v = ray.direction().dot(qVec) * invDet;
  if (v < 0 || u + v > 1)
    return {};

  auto t = vVector().dot(qVec) * invDet;

  if (t < Epsilon)
    return {};

  auto normalUdelta = normals_[1] - normals_[0];
  auto normalVdelta = normals_[2] - normals_[0];
  // TODO: proper barycentric coordinates
  auto normal =
      ((u * normalUdelta) + (v * normalVdelta) + normals_[0]).normalised();
  if (backfacing)
    normal = -normal;
  return Hit{t, ray.positionAlong(t), normal};
}

Triangle::Triangle(const Triangle::Vertices &vertices) : vertices_(vertices) {
  normals_[0] = normals_[1] = normals_[2] = faceNormal();
}
