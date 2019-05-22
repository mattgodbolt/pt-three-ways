#include <math/Camera.h>
#include <math/Sphere.h>
#include <math/Vec3.h>

#include <oo/Primitive.h>
#include <oo/Scene.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

namespace {
bool diffuseOnly = false;
int componentToInt(double x) {
  return lround(pow(std::clamp(x, 0.0, 1.0), 1.0 / 2.2) * 255);
}
} // namespace

template <typename Scene, typename Rng>
Vec3 radiance(const Scene &scene, [[maybe_unused]] Rng &rng, const Ray &ray,
              [[maybe_unused]] int depth) {
  auto intersectionRecord = scene.intersect(ray);
  if (!intersectionRecord)
    return Vec3();

  Material &mat = intersectionRecord->material;
  if (diffuseOnly)
    return mat.diffuse;
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
  // Create a coordinate system local to the point, where the z is the normal at
  // this point.
  const auto basis = OrthoNormalBasis::fromZ(hit.normal);
  // Construct the new direction.
  const auto newDir =
      basis.transform(Vec3(cos(r1) * r2s, sin(r1) * r2s, sqrt(1 - r2)));
  auto newRay = Ray::fromOriginAndDirection(hit.position, newDir.normalised());

  return mat.emission + mat.diffuse * radiance(scene, rng, newRay, depth);
}

template <typename Scene, typename OutputBuffer>
void render(const Scene &scene, OutputBuffer &output, const Camera &camera,
            int numSamples) {
  int width = output.width();
  int height = output.height();

  std::uniform_real_distribution<> unit(0.0, 1.0);
  auto renderPixel = [&](int x, int y) {
    std::minstd_rand rng(x + y * width);
    auto yy = static_cast<double>(y) / height;
    auto xx = static_cast<double>(x) / width;
    Vec3 colour;
    for (int sample = 0; sample < numSamples; ++sample) {
      auto ray = camera.ray(xx, yy, unit(rng), unit(rng));
      colour += radiance(scene, rng, ray, 0);
    }
    return colour * (1.0 / numSamples);
  };

  struct Tile {
    int xBegin;
    int xEnd;
    int yBegin;
    int yEnd;
  };

  struct TileQueue {
    std::mutex mutex;
    size_t numTiles{};
    std::vector<Tile> todo;
    double lastProgress{};

    TileQueue(int width, int height, int xSize, int ySize) {
      for (int y = 0; y < height; y += ySize) {
        int yBegin = y;
        int yEnd = std::min(y + ySize, height);
        for (int x = 0; x < width; x += xSize) {
          int xBegin = x;
          int xEnd = std::min(x + xSize, width);
          todo.emplace_back(Tile{xBegin, xEnd, yBegin, yEnd});
        }
      }
      numTiles = todo.size();
    }

    bool pop(std::function<void(const Tile &)> func) {
      std::unique_lock lock(mutex);
      if (todo.empty())
        return false;
      auto tile = todo.back();
      todo.pop_back();
      lock.unlock();
      func(tile);
      lock.lock();
      auto progress =
          static_cast<double>(numTiles - todo.size()) / numTiles * 100;
      if (progress > lastProgress + 5.) {
        printf("%.2f%%\n", progress);
        lastProgress = progress;
      }
      return true;
    }
  };

  TileQueue queue(width, height, 32, 32);

  auto worker = [&] {
    while (queue.pop([&](const Tile &tile) {
      for (int y = tile.yBegin; y < tile.yEnd; ++y) {
        for (int x = tile.xBegin; x < tile.xEnd; ++x) {
          output.plot(x, y, renderPixel(x, y));
        }
      }
    })) {
      // Do nothing...
    }
  };
  std::vector<std::thread> threads;
  threads.reserve(std::thread::hardware_concurrency());
  for (auto i = 0u; i < std::thread::hardware_concurrency(); ++i) {
    threads.emplace_back(worker);
  }
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

int main() {
  StaticScene scene;
  ArrayOutput output(640, 480);
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
  render(scene, output, camera, 500);

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