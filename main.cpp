#include <math/Sphere.h>
#include <math/Vec3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

namespace {
int componentToInt(double x) {
  return lround(pow(std::clamp(x, 0.0, 1.0), 1.0 / 2.2) * 255 + 0.5);
}
} // namespace

template <typename Scene, typename OutputBuffer>
void render(const Scene &scene, OutputBuffer &output) {
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
      auto xx = (static_cast<double>(x) / width) * 2 - 1.0;
      auto dir =
          (camX * xx * xExtent + camY * yy * yExtent + camDir * screenDistance)
              .normalised();
      auto ray = Ray::fromOriginAndDirection(camPos, dir);
      auto hit = scene.intersect(ray);
      Vec3 colour;
      if (hit) {
        Vec3 mat(1, 0, 1);
        colour = mat * pow(dir.dot(hit->normal), 2);
      }
      output.plot(x, y, colour);
    }
  }
}

namespace {

struct StaticScene {
  Sphere sphere{Vec3(0, 0, 50), 15};
  auto intersect(const Ray &ray) const noexcept {
    return sphere.intersect(ray);
  }
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
  render(scene, output);

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