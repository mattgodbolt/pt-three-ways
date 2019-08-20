#include "Scene.h"
#include "math/OrthoNormalBasis.h"

using dod::Scene;

constexpr double Epsilon = 0.000000001;
constexpr auto FirstBounceNumUSamples = 6;
constexpr auto FirstBounceNumVSamples = 3;

std::optional<dod::IntersectionRecord>
Scene::intersectSpheres(const Ray &ray, double nearerThan) const {
  double currentNearestDist = nearerThan;
  std::optional<size_t> nearestIndex;
  for (size_t sphereIndex = 0; sphereIndex < spheres_.size(); ++sphereIndex) {
    // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
    auto op = spheres_[sphereIndex].centre - ray.origin();
    auto b = op.dot(ray.direction());
    auto determinant =
        b * b - op.lengthSquared() + spheres_[sphereIndex].radiusSquared;
    if (determinant < 0)
      continue;

    determinant = sqrt(determinant);
    static constexpr auto epsilon = 1e-4;
    auto minusT = b - determinant;
    auto plusT = b + determinant;
    if (minusT < epsilon && plusT < epsilon)
      continue;

    auto t = minusT > epsilon ? minusT : plusT;
    if (t < currentNearestDist) {
      nearestIndex = sphereIndex;
      currentNearestDist = t;
    }
  }
  if (!nearestIndex)
    return {};

  auto hitPosition = ray.positionAlong(currentNearestDist);
  auto normal = (hitPosition - spheres_[*nearestIndex].centre).normalised();
  if (normal.dot(ray.direction()) > 0)
    normal = normal * -1;
  return IntersectionRecord{Hit{currentNearestDist, hitPosition, normal},
                            sphereMaterials_[*nearestIndex]};
}

std::optional<dod::IntersectionRecord>
Scene::intersectTriangles(const Ray &ray, double nearerThan) const {
  double currentNearestDist = nearerThan;
  struct Nearest {
    size_t index;
    bool backfacing;
    double u;
    double v;
  };

  std::optional<Nearest> nearest;
  for (size_t i = 0; i < triangleVerts_.size(); ++i) {
    const auto &tv = triangleVerts_[i];
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
  auto &tn = triangleNormals_[nearest->index];
  auto normalUdelta = tn[1] - tn[0];
  auto normalVdelta = tn[2] - tn[0];
  // TODO: proper barycentric coordinates
  auto normal =
      ((nearest->u * normalUdelta) + (nearest->v * normalVdelta) + tn[0])
          .normalised();
  if (nearest->backfacing)
    normal = -normal;
  return IntersectionRecord{
      Hit{currentNearestDist, ray.positionAlong(currentNearestDist), normal},
      triangleMaterials_[nearest->index]};
}

std::optional<dod::IntersectionRecord> Scene::intersect(const Ray &ray) const {
  auto sphereRec =
      intersectSpheres(ray, std::numeric_limits<double>::infinity());
  auto triangleRec = intersectTriangles(
      ray, sphereRec ? sphereRec->hit.distance
                     : std::numeric_limits<double>::infinity());
  return triangleRec ? triangleRec : sphereRec;
}

static constexpr auto MaxDepth = 5;

Vec3 Scene::radiance(std::mt19937 &rng, const Ray &ray, int depth,
                     int numUSamples, int numVSamples, bool preview) const {
  if (depth >= MaxDepth)
    return Vec3();

  const auto intersectionRecord = intersect(ray);
  if (!intersectionRecord)
    return environment_;

  const auto &mat = intersectionRecord->material;
  const auto &hit = intersectionRecord->hit;
  if (preview)
    return mat.diffuse;

  // Sample evenly with random offset.
  std::uniform_real_distribution<> unit(0, 1.0);
  // Create a coordinate system local to the point, where the z is the
  // normal at this point.
  const auto basis = OrthoNormalBasis::fromZ(hit.normal);
  Vec3 result;

  for (auto u = 0; u < numUSamples; ++u) {
    for (auto v = 0; v < numVSamples; ++v) {
      // TODO cone bounce
      auto theta = 2 * M_PI * (static_cast<double>(u) + unit(rng))
                   / static_cast<double>(numUSamples);
      auto radiusSquared = (static_cast<double>(v) + unit(rng))
                           / static_cast<double>(numVSamples);
      auto radius = sqrt(radiusSquared);
      // Construct the new direction.
      const auto newDir =
          basis
              .transform(Vec3(cos(theta) * radius, sin(theta) * radius,
                              sqrt(1 - radiusSquared)))
              .normalised();
      double p = unit(rng);

      if (p < mat.reflectivity) {
        auto reflected =
            ray.direction() - hit.normal * 2 * hit.normal.dot(ray.direction());
        auto newRay = Ray::fromOriginAndDirection(hit.position, reflected);

        result +=
            mat.emission
            + mat.diffuse * radiance(rng, newRay, depth + 1, 1, 1, preview);
      } else {
        auto newRay = Ray::fromOriginAndDirection(hit.position, newDir);

        result +=
            mat.emission
            + mat.diffuse * radiance(rng, newRay, depth + 1, 1, 1, preview);
      }
    }
  }
  if (numUSamples == 1 && numVSamples == 1)
    return result;
  else
    return result * (1.0 / (numUSamples * numVSamples));
}
void Scene::addTriangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2,
                        const Material &material) {
  auto &tv = triangleVerts_.emplace_back(TriangleVertices{v0, v1, v2});
  triangleNormals_.emplace_back(
      TriangleNormals{tv.faceNormal(), tv.faceNormal(), tv.faceNormal()});
  triangleMaterials_.emplace_back(material);
}

void Scene::addSphere(const Vec3 &centre, double radius,
                      const Material &material) {
  spheres_.emplace_back(centre, radius);
  sphereMaterials_.emplace_back(material);
}

void Scene::setEnvironmentColour(const Vec3 &colour) { environment_ = colour; }

ArrayOutput
Scene::render(const Camera &camera, const RenderParams &renderParams,
              const std::function<void(ArrayOutput &output)> &updateFunc) {
  auto width = renderParams.width;
  auto height = renderParams.height;
  ArrayOutput output(width, height);
  std::mt19937 rng(renderParams.samplesPerPixel);
  std::uniform_real_distribution<> unit(0.0, 1.0);

  // TODO no raw loops...maybe return whole "Samples" of an entire screen and
  // accumulate separately? then feeds into a nice multithreaded future based
  // thing?
  // TODO: multi cpus
  for (int sample = 0; sample < renderParams.samplesPerPixel; ++sample) {
    for (auto y = 0; y < height; ++y) {
      for (auto x = 0; x < width; ++x) {
        auto u = unit(rng);
        auto v = unit(rng);
        auto yy = (2 * (static_cast<double>(y) + u + 0.5) / (height - 1)) - 1;
        auto xx = (2 * (static_cast<double>(x) + v + 0.5) / (width - 1)) - 1;
        auto ray = camera.ray(xx, yy, rng);
        output.addSamples(x, y,
                          radiance(rng, ray, 0, FirstBounceNumUSamples,
                                   FirstBounceNumVSamples,
                                   renderParams.preview),
                          1);
      }
    }
    updateFunc(output);
  }
  return output;
}
