#include <math/Sphere.h>
#include <math/Vec3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>

namespace {
int componentToInt(double x) {
  return lround(pow(std::clamp(x, 0.0, 1.0), 1.0 / 2.2) * 255 + 0.5);
}
} // namespace

int main() {
  const int width = 640;
  const int height = 480;
  const auto aspectRatio = static_cast<double>(width) / height;
  const auto yFieldOfView = 0.5135; // TODO work this out, not sure this is fov
  const auto xFieldOfView = yFieldOfView * aspectRatio;

  const Vec3 camPos(0, 0, 0);
  const Vec3 camDir(0, 0, 1);
  const Vec3 camX(1, 0, 0);
  const auto camY = camX.cross(camDir).normalised();

  Sphere sphere(Vec3(0, 0, 50), 15);

  std::unique_ptr<FILE, decltype(fclose) *> f(fopen("image.ppm", "w"), fclose);
  fprintf(f.get(), "P3\n%d %d\n%d\n", width, height, 255);

  for (int y = 0; y < height; ++y) {
    auto yy = (static_cast<double>(y) / height) * 2 - 1.0;
    for (int x = 0; x < width; ++x) {
      auto xx = (static_cast<double>(x) / width) * 2 - 1.0;
      auto dir = (camX * xx * xFieldOfView + camY * yy * yFieldOfView + camDir)
                     .normalised();
      auto ray = Ray::fromOriginAndDirection(camPos, dir);
      auto hit = sphere.intersect(ray);
      Vec3 colour;
      if (hit) {
        Vec3 mat(1,0,1);
        colour = mat * pow(dir.dot(hit->normal), 2);
      }
      fprintf(f.get(), "%d %d %d ", componentToInt(colour.x()),
              componentToInt(colour.y()), componentToInt(colour.z()));
    }
  }

  return 0;
}