#include <algorithm>
#include <math/Camera.h>
#include <math/Sphere.h>
#include <math/Triangle.h>
#include <oo/Material.h>
#include <variant>

namespace fp {
struct Scene {};
class ArrayOutput {
  int width_;
  int height_;
  struct SampledPixel {
    Vec3 colour;
    size_t numSamples{};
    void accumulate(const Vec3 &sample, int num) {
      colour += sample;
      numSamples += num;
    }
    [[nodiscard]] Vec3 result() const {
      if (numSamples == 0)
        return colour;
      return colour * (1.0 / numSamples);
    }
  };
  std::vector<SampledPixel> output_;

public:
  ArrayOutput(int width, int height) : width_(width), height_(height) {
    output_.resize(width * height);
  }
  [[nodiscard]] constexpr int height() const noexcept { return height_; }
  [[nodiscard]] constexpr int width() const noexcept { return width_; }
  void plot(int x, int y, const Vec3 &colour, int numSamples) noexcept {
    output_[x + y * width_].accumulate(colour, numSamples);
  }
  [[nodiscard]] Vec3 pixelAt(int x, int y) const noexcept {
    return output_[x + y * width_].result();
  }
};

#if 0
template <typename Scene, typename Rng>
Vec3 radiance(const Scene &scene, Rng &rng, const Ray &ray, int depth,
              int numUSamples, int numVSamples) {
  auto intersectionRecord = scene.intersect(ray);
  if (!intersectionRecord)
    return scene.environment(ray);

  Material &mat = intersectionRecord->material;
  if (preview)
    return mat.diffuse;
  Hit &hit = intersectionRecord->hit;

  if (++depth > 5) {
    // TODO: "russian roulette"
    return mat.emission;
  }

  Vec3 result;

  // Sample evenly with random offset.
  std::uniform_real_distribution<> unit(0, 1.0);
  for (auto u = 0; u < numUSamples; ++u) {
    for (auto v = 0; v < numVSamples; ++v) {
      auto theta = 2 * M_PI * (static_cast<double>(u) + unit(rng))
                   / static_cast<double>(numUSamples);
      auto radiusSquared = (static_cast<double>(v) + unit(rng))
                           / static_cast<double>(numVSamples);
      auto radius = sqrt(radiusSquared);
      // Create a coordinate system local to the point, where the z is the
      // normal at this point.
      const auto basis = OrthoNormalBasis::fromZ(hit.normal);
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

        result += mat.emission
                  + mat.diffuse * radiance(scene, rng, newRay, depth, 1, 1);
      } else {
        auto newRay = Ray::fromOriginAndDirection(hit.position, newDir);

        result += mat.emission
                  + mat.diffuse * radiance(scene, rng, newRay, depth, 1, 1);
      }
    }
  }
  if (numUSamples == 1 && numVSamples == 1)
    return result;
  else
    return result * (1.0 / (numUSamples * numVSamples));
}
#endif

// What is functional?
// * "monadic" optional hits (would need to remove that from the "OO" version)
// * more free functions than objects (and make `main`'s rendering  type stuff
//   only in the OO.
// * Immutable data structures. e.g. "SampledPixel" in arryayOutput doesn't
//   accumulate, but some final process should. (if we do accumulation)
// * scene intersection code
//   Maybe entire function needs to be passed the parts of a scene, and then
//   builds it itself. "AbstractScene" ? No hierarchy information
// * Scene graph using variant!

struct TriangleElem {
  Triangle triangle;
  Material material;
};
struct SphereElem {
  Sphere sphere;
  Material material;
};

using Elem = std::variant<TriangleElem, SphereElem>;

struct IntersectVisitor {
  const Ray &ray;
  std::optional<Hit> operator()(const TriangleElem &elem) const {
    return elem.triangle.intersect(ray);
  }
  std::optional<Hit> operator()(const SphereElem &elem) const {
    return elem.sphere.intersect(ray);
  }
};


std::optional<Hit> intersect(const Elem &elem, const Ray &ray) {
  return std::visit(IntersectVisitor{ray}, elem);
}

}

#if 0
void render(const Camera &camera, const Scene &scene, const ArrayOutput &output,
            int samplesPerPixel) {
  auto width = output.width();
  auto height = output.height();
  std::mt19937 rng(samplesPerPixel);
  std::uniform_real_distribution<> unit(0.0, 1.0);

  for (auto y = 0; y < height; ++y) {
    for (auto x = 0; x < width; ++x) {
      Vec3 colour;
      for (int sample = 0; sample < samplesPerPixel; ++sample) {
        auto u = unit(rng);
        auto v = unit(rng);
        auto yy = (2 * (static_cast<double>(y) + u + 0.5) / (height - 1)) - 1;
        auto xx = (2 * (static_cast<double>(x) + v + 0.5) / (width - 1)) - 1;
        auto ray = camera.ray(xx, yy, rng);
        colour += radiance(scene, rng, ray, 0, FirstBounceNumUSamples,
                           FirstBounceNumVSamples);
      }
      return colour;
    };
  }
}
}
#endif