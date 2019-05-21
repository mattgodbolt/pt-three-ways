#include <math/Sphere.h>
#include <math/Vec3.h>

#include <oo/Primitive.h>
#include <oo/Scene.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <random>
#include <vector>

namespace {
int componentToInt(double x) {
  return lround(pow(std::clamp(x, 0.0, 1.0), 1.0 / 2.2) * 255 + 0.5);
}
} // namespace

template <typename Scene, typename Rng>
Vec3 radiance(const Scene &scene, Rng &rng, const Ray &ray, int depth) {
  auto intersectionRecord = scene.intersect(ray);
  if (!intersectionRecord)
    return Vec3();

  Material &mat = intersectionRecord->material;
  Hit &hit = intersectionRecord->hit;

  if (++depth > 5) {
    // TODO: "russian roulette"
    return mat.emission;
  }

  // Get a random polar coordinate.
  std::uniform_real_distribution<> polar(0, 2.0 * M_PI);
  auto r1 = polar(rng);
  std::uniform_real_distribution<> unit(0, 1.0);
  auto r2 = unit(rng);
  auto r2s = sqrt(r2);
  // Create a coordinate system u,v,w local to the point, where the w is the
  // normal pointing out of the sphere and the u and v are orthonormal to w.
  const auto &w = hit.normal;
  // Pick an arbitrary non-zero preferred axis for u
  const auto u = (fabs(w.x()) > 0.1 ? Vec3(0, 1, 0) : Vec3(1, 0, 0)).cross(w);
  const auto v = w.cross(u);
  // construct the new direction
  const auto newDir = u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2);
  auto newRay = Ray::fromOriginAndDirection(hit.position, newDir.normalised());

  return mat.emission + mat.diffuse * radiance(scene, rng, newRay, depth);
}

template <typename Scene, typename OutputBuffer>
void render(const Scene &scene, OutputBuffer &output, int numSamples) {
  // TODO: could all be constexpr if outputbuffer is...
  int width = output.width();
  int height = output.height();
  auto aspectRatio = static_cast<double>(width) / height;
  // We assume the screen is suspended "screenDistance" units from the camera
  // position. Looking from the side:
  //        -| -ySize
  //      -- |
  //    --th |
  //  --sD----  0
  //    --   |
  //      -- |
  //        -| ySize
  // th is half field of view angle
  // sD is the screenDistance
  // We need to find yExtent. From geometry, tan(th) = yExtent / sD,
  // so yExtent = sd * tan(th)
  constexpr auto screenDistance = 1.0;
  constexpr auto yFoVDegrees = 50.0;
  auto yExtent = tan(yFoVDegrees / 360 * 2 * M_PI / 2) * screenDistance;
  auto xExtent = yExtent * aspectRatio;

  constexpr Vec3 camPos(0, 0, 0);
  constexpr Vec3 camDir(0, 0, 1);
  constexpr Vec3 camX(1, 0, 0);
  constexpr auto camY = camX.cross(camDir).normalised();

  for (int y = 0; y < height; ++y) {
    auto yy = (static_cast<double>(y) / height) * 2 - 1.0;
    for (int x = 0; x < width; ++x) {
      std::minstd_rand rng(x + y * width);
      auto xx = (static_cast<double>(x) / width) * 2 - 1.0;
      auto dir =
          (camX * xx * xExtent + camY * yy * yExtent + camDir * screenDistance)
              .normalised();
      Vec3 colour;
      for (int sample = 0; sample < numSamples; ++sample) {
        colour +=
            radiance(scene, rng, Ray::fromOriginAndDirection(camPos, dir), 0);
      }
      output.plot(x, y, colour * (1.0 / numSamples));
    }
  }
}

namespace {

struct SpherePrimitive : Primitive {
  Sphere sphere;
  Material material;
  SpherePrimitive(const Sphere &sphere, const Material &material)
      : sphere(sphere), material(material) {}
  std::optional<IntersectionRecord> intersect(const Ray &ray) const override {
    auto hit = sphere.intersect(ray);
    if (!hit)
      return {};
    return IntersectionRecord{*hit, material};
  }
};

struct StaticScene {
  Scene scene;
  StaticScene() {
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(-10, 0, 50), 9.5),
        Material{Vec3(0.1, 0.0, 0.1), Vec3(1, 0, 1)}));
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(10, 0, 50), 9.5), Material{Vec3(), Vec3(1, 1, 0)}));
  }
  auto intersect(const Ray &ray) const noexcept { return scene.intersect(ray); }
};

class ArrayOutput {
  int width_;
  int height_;
  std::vector<Vec3> output_;

public:
  ArrayOutput(int width, int height) : width_(width), height_(height) {
    output_.resize(width * height);
  }
  constexpr int height() const noexcept { return height_; }
  constexpr int width() const noexcept { return width_; }
  void plot(int x, int y, const Vec3 &colour) noexcept {
    output_[x + y * width_] = colour;
  }
  const Vec3 &pixelAt(int x, int y) noexcept { return output_[x + y * width_]; }
};

} // namespace

int main() {
  StaticScene scene;
  ArrayOutput output(320, 240);
  render(scene, output, 1024);

  std::unique_ptr<FILE, decltype(fclose) *> f(fopen("image.ppm", "w"), fclose);
  auto width = output.width();
  auto height = output.height();
  fprintf(f.get(), "P3\n%d %d\n%d\n", width, height, 255);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      auto &colour = output.pixelAt(x, y);
      fprintf(f.get(), "%d %d %d ", componentToInt(colour.x()),
              componentToInt(colour.y()), componentToInt(colour.z()));
    }
  }
}