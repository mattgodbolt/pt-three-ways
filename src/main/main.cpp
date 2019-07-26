#include "WorkQueue.h"

#include <math/Camera.h>
#include <math/Sphere.h>
#include <math/Vec3.h>

#include <oo/Primitive.h>
#include <oo/Scene.h>

#include <clara.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <math/Triangle.h>
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
  int samples;
};

std::vector<Tile> generateTiles(int width, int height, int xSize, int ySize,
                                int numSamples, int samplesPerTile) {
  std::vector<Tile> tiles;
  for (int y = 0; y < height; y += ySize) {
    int yBegin = y;
    int yEnd = std::min(y + ySize, height);
    for (int x = 0; x < width; x += xSize) {
      int xBegin = x;
      int xEnd = std::min(x + xSize, width);
      for (int s = 0; s < numSamples; s += samplesPerTile) {
        int nSamples = std::min(s + samplesPerTile, numSamples);
        tiles.emplace_back(Tile{xBegin, xEnd, yBegin, yEnd, nSamples});
      }
    }
  }
  std::mt19937 rng(25115284);
  std::shuffle(tiles.begin(), tiles.end(), rng);
  return tiles;
}

}

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
      auto theta = 2 * M_PI * (static_cast<double>(u) + unit(rng))
                   / static_cast<double>(sqrtSamples);
      auto radiusSquared = (static_cast<double>(v) + unit(rng))
                           / static_cast<double>(sqrtSamples);
      auto radius = sqrt(radiusSquared);
      // Create a coordinate system local to the point, where the z is the
      // normal at this point.
      const auto basis = OrthoNormalBasis::fromZ(hit.normal);
      // Construct the new direction.
      const auto newDir = basis.transform(Vec3(
          cos(theta) * radius, sin(theta) * radius, sqrt(1 - radiusSquared)));
      auto newRay =
          Ray::fromOriginAndDirection(hit.position, newDir.normalised());

      result +=
          mat.emission
          + mat.diffuse * radiance(scene, rng, newRay, depth, nextSqrtSamples);
    }
  }
  if (sqrtSamples == 1)
    return result;
  else
    return result * (1.0 / (sqrtSamples * sqrtSamples));
}

template <typename Scene, typename OutputBuffer, typename Flush>
void render(const Scene &scene, OutputBuffer &output, const Camera &camera,
            int samplesPerPixel, unsigned numThreads, Flush flush) {
  int width = output.width();
  int height = output.height();

  std::uniform_real_distribution<> unit(0.0, 1.0);
  auto renderPixel = [&](std::mt19937 &rng, int x, int y, int samples) {
    auto yy = static_cast<double>(y) / height;
    auto xx = static_cast<double>(x) / width;
    Vec3 colour;
    for (int sample = 0; sample < samples; ++sample) {
      auto ray = camera.ray(xx, yy, unit(rng), unit(rng));
      colour += radiance(scene, rng, ray, 0, SqrtFirstBounceSamples);
    }
    return colour;
  };

  WorkQueue<Tile> queue(
      generateTiles(width, height, 16, 16, samplesPerPixel, 8));

  auto worker = [&] {
    for (;;) {
      auto tileOpt = queue.pop(flush);
      if (!tileOpt)
        break;
      auto &tile = *tileOpt;

      std::mt19937 rng(tile.xBegin + tile.yBegin * width);
      for (int y = tile.yBegin; y < tile.yEnd; ++y) {
        for (int x = tile.xBegin; x < tile.xEnd; ++x) {
          output.plot(x, y, renderPixel(rng, x, y, tile.samples), tile.samples);
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
  [[nodiscard]] std::optional<IntersectionRecord>
  intersect(const Ray &ray) const override {
    auto hit = sphere.intersect(ray);
    if (!hit)
      return {};
    return IntersectionRecord{*hit, material};
  }
};

struct BoxPrimitive : Primitive {
  std::array<Triangle, 8> triangles;
  Material material;
  static constexpr Vec3 V(const Vec3 &centre, const Vec3 &size, bool x, bool y,
                          bool z) {
    return centre + size * Vec3(x ? 1 : -1, y ? 1 : -1, z ? 1 : -1);
  }
  BoxPrimitive(const Vec3 &centre, const Vec3 &size, const Material &material)
      // TODO these are all backwards and the wrong winding order etc...
      : triangles({
          Triangle(V(centre, size, false, true, false),
                   V(centre, size, true, true, false),
                   V(centre, size, false, false, false)),
          Triangle(V(centre, size, true, true, false),
                   V(centre, size, false, false, false),
                   V(centre, size, true, false, false)),
          Triangle(V(centre, size, false, true, true),
                   V(centre, size, true, true, true),
                   V(centre, size, false, false, true)),
          Triangle(V(centre, size, true, true, true),
                   V(centre, size, false, false, true),
                   V(centre, size, true, false, true)),
          // todo not sure this is right (maybe I should write a mesh loader
          // instead of this...
          Triangle(V(centre, size, false, false, true),
                   V(centre, size, true, false, true),
                   V(centre, size, false, false, false)),
          Triangle(V(centre, size, true, false, true),
                   V(centre, size, false, false, false),
                   V(centre, size, true, false, false)),
          Triangle(V(centre, size, false, false, true),
                   V(centre, size, true, false, true),
                   V(centre, size, false, false, false)),
          Triangle(V(centre, size, true, true, true),
                   V(centre, size, false, true, false),
                   V(centre, size, true, true, false)),
      }),
        material(material) {}
  std::optional<IntersectionRecord> intersect(const Ray &ray) const override {
    std::optional<Hit> nearestHit;
    for (auto &&t : triangles) {
      auto hit = t.intersect(ray);
      if (hit && (!nearestHit || hit->distance < nearestHit->distance))
        nearestHit = hit;
    }
    if (!nearestHit)
      return {};
    return IntersectionRecord{*nearestHit, material};
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

    scene.add(std::make_unique<BoxPrimitive>(
        Vec3(27, 16.5, 47), Vec3(16.5, 16.5, 16.5),
        Material::makeDiffuse(Vec3(0.999, 0.999, 0.999))));
    //    scene.add(std::make_unique<SpherePrimitive>(
    //        Sphere(Vec3(27, 16.5, 47), 16.5),
    //        Material::makeDiffuse(Vec3(0.999, 0.999, 0.999))));
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(73, 16.5, 78), 16.5),
        Material::makeDiffuse(Vec3(0.999, 0.999, 0.999))));
  }
  auto intersect(const Ray &ray) const noexcept { return scene.intersect(ray); }
};

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

}

int main(int argc, const char *argv[]) {
  StaticScene scene;

  bool help = false;
  auto width = 1920;
  auto height = 1080;
  auto numCpus = 1u;
  auto samplesPerPixel = 40;

  using namespace clara;
  auto cli =
      Opt(width, "width")["-w"]["--width"]("output image width")
      | Opt(height, "height")["-h"]["--height"]("output image height")
      | Opt(numCpus,
            "numCpus")["--num-cpus"]("number of CPUs to use (0 for all)")
      | Opt(samplesPerPixel, "samples")["--spp"]("number of samples per pixel")
      | Opt(preview)["--preview"]("super quick preview") | Help(help);

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

  auto save = [&]() {
    std::unique_ptr<FILE, decltype(fclose) *> f(fopen("image.ppm", "w"),
                                                fclose);
    fprintf(f.get(), "P3\n%d %d\n%d\n", width, height, 255);
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        auto colour = output.pixelAt(x, y);
        fprintf(f.get(), "%d %d %d ", componentToInt(colour.x()),
                componentToInt(colour.y()), componentToInt(colour.z()));
      }
    }
  };

  using namespace std::literals;
  static constexpr auto saveEvery = 5s;
  auto nextSave = std::chrono::system_clock::now() + saveEvery;
  render(scene, output, camera, samplesPerPixel, numCpus, [&] {
    // TODO: save is not thread safe even slightly, and yet it still blocks the
    // threads. this is terrible. Should have a thread safe result queue and
    // a single thread reading from it.
    auto now = std::chrono::system_clock::now();
    if (now > nextSave) {
      save();
      nextSave = now + saveEvery;
    }
  });

  save();
}