#include "Render.h"

#include "Primitive.h"
#include "Random2DSampler.h"
#include "Scene.h"
#include "math/Camera.h"
#include "math/Samples.h"
#include "util/ArrayOutput.h"
#include "util/Progressifier.h"

#include <future>
#include <range/v3/all.hpp>

namespace fp {

constexpr auto FirstBounceNumUSamples = 6;
constexpr auto FirstBounceNumVSamples = 3;

// What is functional?
// * Immutable data structures. e.g. "SampledPixel" in arrayOutput doesn't
//   accumulate, but some final process should. (if we do accumulation)
// * Scene graph using variant!
// * TMP stuff (if can find a way)

struct IntersectionRecord {
  Hit hit;
  Material material;
};

struct IntersectVisitor {
  const Ray &ray;

  static std::optional<IntersectionRecord>
  unwrapWith(const std::optional<Hit> &hit, const Material &m) {
    if (!hit)
      return {};
    return IntersectionRecord{*hit, m};
  }

  std::optional<IntersectionRecord>
  operator()(const TrianglePrimitive &primitive) const {
    return unwrapWith(primitive.triangle.intersect(ray), primitive.material);
  }
  std::optional<IntersectionRecord>
  operator()(const SpherePrimitive &primitive) const {
    return unwrapWith(primitive.sphere.intersect(ray), primitive.material);
  }
};

std::optional<IntersectionRecord> intersect(const Primitive &primitive,
                                            const Ray &ray) {
  return std::visit(IntersectVisitor{ray}, primitive);
}

std::optional<IntersectionRecord> intersect(const Scene &scene,
                                            const Ray &ray) {
  std::optional<IntersectionRecord> nearest;
  for (auto &primitive : scene.primitives) {
    auto thisIntersection = intersect(primitive, ray);
    if (thisIntersection
        && (!nearest || thisIntersection->hit.distance < nearest->hit.distance))
      nearest = thisIntersection;
  }
  return nearest;
}

Vec3 radiance(const Scene &scene, std::mt19937 &rng, const Ray &ray, int depth,
              int numUSamples, int numVSamples, bool preview);

Vec3 singleRay(const Scene &scene, std::mt19937 &rng,
               const IntersectionRecord &intersectionRecord, const Ray &ray,
               const OrthoNormalBasis &basis, double u, double v, int depth,
               bool preview) {
  std::uniform_real_distribution<> unit(0, 1.0);
  const auto &mat = intersectionRecord.material;
  const auto &hit = intersectionRecord.hit;
  const auto p = unit(rng);

  if (p < mat.reflectivity) {
    auto newRay =
        Ray(hit.position, coneSample(hit.normal.reflect(ray.direction()),
                                     mat.reflectionConeAngle(), u, v));

    return radiance(scene, rng, newRay, depth, 1, 1, preview);
  } else {
    auto newRay = Ray(hit.position, hemisphereSample(basis, u, v));

    return radiance(scene, rng, newRay, depth, 1, 1, preview);
  }
}

static constexpr auto MaxDepth = 5;
Vec3 radiance(const Scene &scene, std::mt19937 &rng, const Ray &ray, int depth,
              int numUSamples, int numVSamples, bool preview) {
  if (depth >= MaxDepth)
    return Vec3();
  const auto intersectionRecord = intersect(scene, ray);
  if (!intersectionRecord)
    return scene.environment;

  const auto &mat = intersectionRecord->material;
  const auto &hit = intersectionRecord->hit;
  if (preview)
    return mat.diffuse;

  // Create a coordinate system local to the point, where the z is the
  // normal at this point.
  const auto basis = OrthoNormalBasis::fromZ(hit.normal);
  const auto sampleScale = 1.0 / (numUSamples * numVSamples);
  const auto range = Random2DSampler(rng, numUSamples, numVSamples);
  const Vec3 incomingLight = std::accumulate(
      std::begin(range), std::end(range), Vec3(),
      [&](Vec3 colour, const std::pair<double, double> &s) {
        return colour
               + singleRay(scene, rng, *intersectionRecord, ray, basis, s.first,
                           s.second, depth + 1, preview);
      });
  return mat.emission + mat.diffuse * incomingLight * sampleScale;
}

ArrayOutput renderWholeScreen(const Camera &camera, const Scene &scene,
                              size_t seed, const RenderParams &renderParams) {
  using namespace ranges;
  auto renderOnePixel = [seed, &renderParams, &camera, &scene](auto tuple) {
    auto [y, x] = tuple;
    std::mt19937 rng(renderParams.height * renderParams.width * seed
                     + x * renderParams.width + y);
    return radiance(scene, rng, camera.randomRay(x, y, rng), 0,
                    FirstBounceNumUSamples, FirstBounceNumVSamples,
                    renderParams.preview);
  };
  auto renderedPixelsView =
      view::cartesian_product(view::ints(0, renderParams.height),
                              view::ints(0, renderParams.width))
      | view::transform(renderOnePixel);
  return ArrayOutput(renderParams.width, renderParams.height,
                     renderedPixelsView);
}

ArrayOutput render(const Camera &camera, const Scene &scene,
                   const RenderParams &renderParams,
                   const std::function<void(const ArrayOutput &)> &updateFunc) {
  // TODO no raw loops...maybe return whole "Samples" of an entire screen and
  // accumulate separately?
  // future from an async()
  size_t seed = 0;
  size_t numDone = 0;
  ArrayOutput output(renderParams.width, renderParams.height);
  Progressifier progressifier(renderParams.samplesPerPixel);
  for (int sample = 0; sample < renderParams.samplesPerPixel;
       sample += renderParams.maxCpus) {
    std::vector<std::future<ArrayOutput>> futures;
    for (int ss = sample; ss < std::min(renderParams.samplesPerPixel,
                                        sample + renderParams.maxCpus);
         ++ss) {
      futures.emplace_back(std::async(std::launch::async, [&] {
        return renderWholeScreen(camera, scene, seed++, renderParams);
      }));
    }
    for (auto &&future : futures) {
      future.wait();
      output += future.get();
      numDone++;
      progressifier.update(numDone);
      updateFunc(output);
    }
  }
  return output;
}

}