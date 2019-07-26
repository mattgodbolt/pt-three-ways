#pragma once

#include "math/Hit.h"
#include "math/Ray.h"
#include "oo/Material.h"

#include <optional>
#include <utility>

class Primitive {
public:
  virtual ~Primitive() = default;
  struct IntersectionRecord {
    Hit hit;
    Material material;
  };

  [[nodiscard]] virtual std::optional<IntersectionRecord>
  intersect(const Ray &ray) const = 0;
};
