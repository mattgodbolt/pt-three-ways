#include "Scene.h"

using oo::Scene;
using oo::Primitive;

std::optional<Primitive::IntersectionRecord>
Scene::intersect(const Ray &ray) const {
  std::optional<Primitive::IntersectionRecord> currentNearest;
  for (auto &primitive : primitives_) {
    auto intersection = primitive->intersect(ray);
    if (!intersection)
      continue;
    if (!currentNearest
        || intersection->hit.distance < currentNearest->hit.distance) {
      currentNearest = intersection;
    }
  }
  return currentNearest;
}

void Scene::add(std::unique_ptr<Primitive> primitive) {
  primitives_.emplace_back(std::move(primitive));
}
