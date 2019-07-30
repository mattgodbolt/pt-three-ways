#include "PngWriter.h"
#include "WorkQueue.h"

#include <math/Camera.h>
#include <math/Vec3.h>
#include <oo/Sphere.h>

#include <oo/Primitive.h>
#include <oo/Scene.h>

#include <clara.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <oo/ObjLoader.h>
#include <oo/Triangle.h>
#include <random>
#include <thread>
#include <utility>
#include <vector>

using namespace oo; // TODO not this. extract renderer from OO and make "OOey"

namespace {

constexpr auto FirstBounceNumUSamples = 6;
constexpr auto FirstBounceNumVSamples = 3;
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
  int sampleNum;
  size_t distancePrio;
  size_t randomPrio;
  [[nodiscard]] constexpr auto key() const {
    return std::tie(sampleNum, distancePrio, randomPrio);
  }
};

std::vector<Tile> generateTiles(int width, int height, int xSize, int ySize,
                                int numSamples, int samplesPerTile) {
  std::mt19937 rng(width * height);
  std::vector<Tile> tiles;
  for (int y = 0; y < height; y += ySize) {
    int yBegin = y;
    int yEnd = std::min(y + ySize, height);
    for (int x = 0; x < width; x += xSize) {
      int xBegin = x;
      int xEnd = std::min(x + xSize, width);
      int midX = (xEnd + xBegin) / 2;
      int midY = (yEnd + yBegin) / 2;
      int centreX = width / 2;
      int centreY = height / 2;
      size_t distanceSqr = (midX - centreX) * (midX - centreX)
                           + (midY - centreY) * (midY - centreY);
      for (int s = 0; s < numSamples; s += samplesPerTile) {
        int nSamples = std::min(s + samplesPerTile, numSamples);
        tiles.emplace_back(
            Tile{xBegin, xEnd, yBegin, yEnd, nSamples, s, distanceSqr, rng()});
      }
    }
  }
  std::sort(tiles.begin(), tiles.end(), [](const Tile &lhs, const Tile &rhs) {
    return lhs.key() > rhs.key();
  });
  return tiles;
}

}

template <typename Scene, typename Rng>
Vec3 radiance(const Scene &scene, Rng &rng, const Ray &ray, int depth,
              int numUSamples, int numVSamples) {
  Primitive::IntersectionRecord intersectionRecord;
  if (!scene.intersect(ray, intersectionRecord))
    return scene.environment(ray);

  Material &mat = intersectionRecord.material;
  if (preview)
    return mat.diffuse;
  Hit &hit = intersectionRecord.hit;

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

template <typename Scene, typename OutputBuffer, typename Flush>
void render(const Scene &scene, OutputBuffer &output, const Camera &camera,
            int samplesPerPixel, unsigned numThreads, Flush flush) {
  int width = output.width();
  int height = output.height();

  std::uniform_real_distribution<> unit(0.0, 1.0);
  auto renderPixel = [&](std::mt19937 &rng, int x, int y, int samples) {
    Vec3 colour;
    for (int sample = 0; sample < samples; ++sample) {
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

  WorkQueue<Tile> queue(
      generateTiles(width, height, 16, 16, samplesPerPixel, 8));

  auto worker = [&] {
    for (;;) {
      auto tileOpt = queue.pop(flush);
      if (!tileOpt)
        break;
      auto &tile = *tileOpt;

      std::mt19937 rng(tile.randomPrio);
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
  [[nodiscard]] bool intersect(const Ray &ray,
                               IntersectionRecord &rec) const override {
    Hit hit;
    if (!sphere.intersect(ray, hit))
      return false;
    rec = IntersectionRecord{hit, material};
    return true;
  }
};

struct DirRelativeOpener : ObjLoaderOpener {
  std::string dir_;
  explicit DirRelativeOpener(std::string dir) : dir_(std::move(dir)) {}
  [[nodiscard]] std::unique_ptr<std::istream>
  open(const std::string &filename) override {
    auto fullname = dir_ + "/" + filename;
    auto res = std::make_unique<std::ifstream>(fullname);
    if (!*res)
      throw std::runtime_error("Unable to open " + fullname);
    return res;
  }
};

struct ObjPrimitive : Primitive {
  ObjFile obj;
  explicit ObjPrimitive(ObjFile o) : obj(std::move(o)) {}
  [[nodiscard]] bool
  intersect(const Ray &ray,
            IntersectionRecord &intersectionRecord) const override {
    bool hasHit = false;
    Hit nearestHit;
    size_t hitIndex{};
    for (size_t index = 0; index < obj.materials.size(); ++index) {
      auto &t = obj.triangles[index];
      Hit hit;
      if (t.intersect(ray, hit)
          && (!hasHit || hit.distance < nearestHit.distance)) {
        hasHit = true;
        nearestHit = hit;
        hitIndex = index;
      }
    }
    if (!hasHit)
      return {};
    intersectionRecord =
        IntersectionRecord{nearestHit, obj.materials[hitIndex]};
    return true;
  }
};

struct DodgyCornellScene {
  Scene scene;
  DodgyCornellScene() {
    DirRelativeOpener opener("scenes");
    auto in = opener.open("CornellBox-Original.obj");
    auto obj = loadObjFile(*in, opener);
    scene.add(std::make_unique<ObjPrimitive>(obj));
    scene.add(std::make_unique<SpherePrimitive>(
        Sphere(Vec3(-0.38, 0.281, 0.38), 0.28),
        Material::makeReflective(Vec3(0.999, 0.999, 0.999), 0.75)));
  }
  [[nodiscard]] auto intersect(const Ray &ray,
                               Primitive::IntersectionRecord &rec) const
      noexcept {
    return scene.intersect(ray, rec);
  }
  [[nodiscard]] auto environment(const Ray &) const noexcept {
    return Vec3(0.725, 0.71, 0.68) * 0.1;
  }
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
  DodgyCornellScene scene;

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
  //  Vec3 camPos(50, 52, 295.6);
  //  Vec3 camUp(0, -1, 0);
  //  auto camDir = Vec3(0, -0.042612, -1).normalised();
  Vec3 camPos(0, 1, 3);
  Vec3 camUp(0, 1, 0);
  Vec3 camLookAt(0, 1, 0);
  double verticalFov = 50.0;
  Camera camera(camPos, camLookAt, camUp, width, height, verticalFov);
  camera.setFocus(Vec3(0, 0, 0), 0.01);

  auto save = [&]() {
    PngWriter pw("image.png", width, height);
    if (!pw.ok()) {
      std::cerr << "Unable to save PNG\n";
      return;
    }

    for (int y = 0; y < height; ++y) {
      std::uint8_t row[width * 3];
      for (int x = 0; x < width; ++x) {
        auto colour = output.pixelAt(x, y);
        row[x * 3] = componentToInt(colour.x());
        row[x * 3 + 1] = componentToInt(colour.y());
        row[x * 3 + 2] = componentToInt(colour.z());
      }
      pw.addRow(row);
    }
  };

  using namespace std::literals;
  static constexpr auto saveEvery = 10s;
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