#include "math/Vec3.h"
#include "util/Material.h"

#include <array>
#include <math/Hit.h>
#include <math/Ray.h>
#include <optional>
#include <random>
#include <vector>

namespace dod {

class TriangleVertices {
public:
  using Vertices = std::array<Vec3, 3>;

private:
  Vertices vertices_;

public:
  explicit TriangleVertices(const Vertices &vertices) : vertices_(vertices) {}
  TriangleVertices(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2)
      : TriangleVertices(Vertices{v0, v1, v2}) {}

  [[nodiscard]] constexpr const Vec3 &vertex(int index) const {
    return vertices_[index];
  }

  [[nodiscard]] constexpr Vec3 uVector() const {
    return vertices_[1] - vertices_[0];
  }

  [[nodiscard]] constexpr Vec3 vVector() const {
    return vertices_[2] - vertices_[0];
  }

  [[nodiscard]] constexpr Vec3 faceNormal() const {
    return uVector().cross(vVector()).normalised();
  }
};

using TriangleNormals = std::array<Vec3, 3>;

constexpr double Epsilon = 0.000000001;

struct Scene {
  std::vector<TriangleVertices> triangleVerts;
  std::vector<TriangleNormals> triangleNormals;
  std::vector<Material> triangleMaterials;

  std::vector<Vec3> sphereCentres;
  std::vector<double> sphereRadii;
  std::vector<Material> sphereMaterials;

  std::optional<Hit> intersectTriangles(const Ray &ray,
                                        double nearerThan) const {
    double currentNearestDist = nearerThan;
    struct Nearest {
      size_t index;
      bool backfacing;
      double u;
      double v;
    };

    std::optional<Nearest> nearest;
    for (size_t i = 0; i < triangleVerts.size(); ++i) {
      const auto &tv = triangleVerts[i];
      auto pVec = ray.direction().cross(tv.vVector());
      auto det = tv.uVector().dot(pVec);
      // ray and triangle are parallel if det is close to 0
      if (fabs(det) < Epsilon)
        continue;

      auto invDet = 1.0 / det;
      auto tVec = ray.origin() - tv.vertex(0);
      auto u = tVec.dot(pVec) * invDet;
      if (u < 0.0 || u > 1.0)
        continue;

      auto qVec = tVec.cross(tv.uVector());
      auto v = ray.direction().dot(qVec) * invDet;
      if (v < 0 || u + v > 1)
        continue;

      auto t = tv.vVector().dot(qVec) * invDet;

      if (t > Epsilon && t < currentNearestDist) {
        nearest = Nearest{i, det < Epsilon, u, v};
        currentNearestDist = t;
      }
    }
    if (!nearest)
      return {};
    auto &tn = triangleNormals[nearest->index];
    auto normalUdelta = tn[1] - tn[0];
    auto normalVdelta = tn[2] - tn[0];
    // TODO: proper barycentric coordinates
    auto normal =
        ((nearest->u * normalUdelta) + (nearest->v * normalVdelta) + tn[0])
            .normalised();
    if (nearest->backfacing)
      normal = -normal;
    return Hit{currentNearestDist, ray.positionAlong(currentNearestDist),
               normal};
  }
};

// Vec3 radiance(const Scene &scene, std::mt19937 &rng, const Ray &ray, int
// depth,
//              int numUSamples, int numVSamples, bool preview) {
//  //
//  const auto intersectionRecord = intersect(scene, ray);
//  if (!intersectionRecord)
//    return scene.environment;
//}

}