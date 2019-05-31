#include "WorkQueue.h"

#include <math/Camera.h>
#include <math/Sphere.h>
#include <math/Vec3.h>

#include <oo/Primitive.h>
#include <oo/Scene.h>

#include <clara.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

namespace {

constexpr auto SqrtFirstBounceSamples = 4;
bool preview = false;
int componentToInt(double x) {
  return lround(pow(std::clamp(x, 0.0, 1.0), 1.0 / 2.2) * 255);
}

struct Tile {
  int xBegin;
  int xEnd;
  int yBegin;
  int yEnd;
};

std::vector<Tile> generateTiles(int width, int height, int xSize, int ySize) {
  std::vector<Tile> tiles;
  for (int y = 0; y < height; y += ySize) {
    int yBegin = y;
    int yEnd = std::min(y + ySize, height);
    for (int x = 0; x < width; x += xSize) {
      int xBegin = x;
      int xEnd = std::min(x + xSize, width);
      tiles.emplace_back(Tile{xBegin, xEnd, yBegin, yEnd});
    }
  }
  return tiles;
}

} // namespace

template <typename Scene, typename Rng>
Vec3 radiance(const Scene &scene, Rng &rng, const Ray &ray, int depth,
              int sqrtSamples) {
  auto intersectionRecord = scene.intersect(ray);
  if (!intersectionRecord)
    return Vec3();

  Material &mat = intersectionRecord->material;
  if (preview)
    return mat.diffuse;
  Hit &hit = intersectionRecord->hit;

  if (++depth > 5) {
    // TODO: "russian roulette"
    return mat.emission;
  }

  Vec3 result;

  const auto nextSqrtSamples = 1;

  // Sample evenly over sqrtSamples x sqrtSamples, with random offset.
  std::uniform_real_distribution<> unit(0, 1.0);
  for (auto u = 0; u < sqrtSamples; ++u) {
    for (auto v = 0; v < sqrtSamples; ++v) {
      auto theta = 2 * M_PI * (static_cast<double>(u) + unit(rng)) /
                   static_cast<double>(sqrtSamples);
      auto radiusSquared = (static_cast<double>(v) + unit(rng)) /
                           static_cast<double>(sqrtSamples);
      auto radius = sqrt(radiusSquared);
      // Create a coordinate system local to the point, where the z is the
      // normal at this point.
      const auto basis = OrthoNormalBasis::fromZ(hit.normal);
      // Construct the new direction.
      const auto newDir = basis.transform(Vec3(
          cos(theta) * radius, sin(theta) * radius, sqrt(1 - radiusSquared)));
      auto newRay =
          Ray::fromOriginAndDirection(hit.position, newDir.normalised());

      result += mat.emission + mat.diffuse * radiance(scene, rng, newRay, depth,
                                                      nextSqrtSamples);
    }
  }
  if (sqrtSamples == 1)
    return result;
  else
    return result * (1.0 / (sqrtSamples * sqrtSamples));
}

template <typename Scene, typename OutputBuffer>
void render(const Scene &scene, OutputBuffer &output, const Camera &camera,
            int samplesPerPixel, unsigned numThreads) {
  int width = output.width();
  int height = output.height();

  std::uniform_real_distribution<> unit(0.0, 1.0);
  auto renderPixel = [&](int x, int y) {
    std::mt19937 rng(x + y * width);
    auto yy = static_cast<double>(y) / height;
    auto xx = static_cast<double>(x) / width;
    Vec3 colour;
    for (int sample = 0; sample < samplesPerPixel; ++sample) {
      auto ray = camera.ray(xx, yy, unit(rng), unit(rng));
      colour += radiance(scene, rng, ray, 0, SqrtFirstBounceSamples);
    }
    return colour * (1.0 / samplesPerPixel);
  };

  WorkQueue<Tile> queue(generateTiles(width, height, 32, 32));

  auto worker = [&] {
    for (;;) {
      auto tileOpt = queue.pop();
      if (!tileOpt)
        break;
      auto &tile = *tileOpt;
      for (int y = tile.yBegin; y < tile.yEnd; ++y) {
        for (int x = tile.xBegin; x < tile.xEnd; ++x) {
          output.plot(x, y, renderPixel(x, y));
        }
      }
    }
  };
  std::vector<std::thread> threads{numThreads};
  std::generate(threads.begin(), threads.end(),
                [&]() { return std::thread(worker); });
  for (auto &t : threads)
    t.join();
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
    // left
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(1e5 + 1, 40.8, 81.6), 1e5),
        Material::makeDiffuse(Vec3(0.75, 0.25, 0.25))));
    // right
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(-1e5 + 99, 40.8, 81.6), 1e5),
        Material::makeDiffuse(Vec3(0.25, 0.25, 0.75))));
    // back
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(50, 40.8, 1e5), 1e5),
        Material::makeDiffuse(Vec3(0.75, 0.75, 0.75))));
    // front
    //    scene.add(std::make_unique<SpherePrimitive>(
    //        Sphere(Vec3(50, 40.8, -1e5 + 170), 1e5),
    //        Material::makeDiffuse(Vec3())));
    // bottom
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(50, 1e5, 81.6), 1e5),
        Material::makeDiffuse(Vec3(0.75, 0.75, 0.75))));
    // top
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(50, -1e5 + 81.6, 81.6), 1e5),
        Material::makeDiffuse(Vec3(0.75, 0.75, 0.75))));
    // light
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(50, 681.6 - .27, 81.6), 600),
        Material{Vec3(12, 12, 12), Vec3()}));

    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(27, 16.5, 47), 16.5),
        Material::makeDiffuse(Vec3(0.999, 0.999, 0.999))));
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(73, 16.5, 78), 16.5),
        Material::makeDiffuse(Vec3(0.999, 0.999, 0.999))));
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

int main(int argc, const char *argv[]) {
  StaticScene scene;

  bool help = false;
  auto width = 1920;
  auto height = 1080;
  auto numCpus = 1u;
  auto samplesPerPixel = 40;

  using namespace clara;
  auto cli =
      Opt(width, "width")["-w"]["--width"]("output image width") |
      Opt(height, "height")["-h"]["--height"]("output image height") |
      Opt(numCpus,
          "numCpus")["--num-cpus"]("number of CPUs to use (0 for all)") |
      Opt(samplesPerPixel, "samples")["--spp"]("number of samples per pixel") |
      Opt(preview)["--preview"]("super quick preview") | Help(help);

  auto result = cli.parse(Args(argc, argv));
  if (!result) {
    std::cerr << "Error in command line: " << result.errorMessage() << '\n';
    exit(1);
  }
  if (help) {
    std::cout << cli;
    exit(0);
  }

  if (numCpus == 0) {
    numCpus = std::thread::hardware_concurrency();
  }

  ArrayOutput output(width, height);
  Vec3 camPos(50, 52, 295.6);
  Vec3 camUp(0, -1, 0);
  auto camDir = Vec3(0, -0.042612, -1).normalised();
  double aspectRatio = static_cast<double>(output.width()) / output.height();
  double verticalFov = 50.0;
  double camHeight = 1.0;
  double camWidth = camHeight * aspectRatio;
  double aperture = 0.0; // Pinhole camera
  double distance = camHeight / tan(verticalFov / 2. / 360 * 2 * M_PI);
  Camera camera(camPos, camDir, camUp, aperture, -camWidth / 2, camWidth / 2,
                -camHeight / 2, camHeight / 2, distance);
  render(scene, output, camera, samplesPerPixel, numCpus);

  std::unique_ptr<FILE, decltype(fclose) *> f(fopen("image.ppm", "w"), fclose);
  fprintf(f.get(), "P3\n%d %d\n%d\n", width, height, 255);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      auto &colour = output.pixelAt(x, y);
      fprintf(f.get(), "%d %d %d ", componentToInt(colour.x()),
              componentToInt(colour.y()), componentToInt(colour.z()));
    }
  }
}