#include "SceneBuilder.h"
#include "Primitive.h"
#include "Sphere.h"
#include "Triangle.h"

using oo::SceneBuilder;

namespace oo {
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

struct TrianglePrimitive : Primitive {
  Triangle triangle;
  Material material;
  explicit TrianglePrimitive(const Triangle &triangle, const Material &material)
      : triangle(triangle), material(material) {}
  [[nodiscard]] bool
  intersect(const Ray &ray,
            IntersectionRecord &intersectionRecord) const override {
    Hit hit;
    if (!triangle.intersect(ray, hit))
      return false;
    intersectionRecord = IntersectionRecord{hit, material};
    return true;
  }
};

}
}

void SceneBuilder::addTriangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2,
                               const ::Material &material) {
  scene_.add(std::make_unique<TrianglePrimitive>(Triangle(v0, v1, v2),
                                                 Material(material)));
}
void SceneBuilder::addSphere(const Vec3 &centre, double radius,
                             const ::Material &material) {
  scene_.add(std::make_unique<SpherePrimitive>(Sphere(centre, radius),
                                               Material(material)));
}

void SceneBuilder::setEnvironmentColour(const Vec3 &colour) {
  scene_.setEnvironmentColour(colour);
}
