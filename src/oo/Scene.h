#pragma once

#include "oo/Primitive.h"

#include <memory>
#include <vector>

class Scene : public Primitive {
  std::vector<std::unique_ptr<Primitive>> primitives_;

public:
  void add(std::unique_ptr<Primitive> primitive);

  [[nodiscard]] std::optional<IntersectionRecord>
  intersect(const Ray &ray) const override;
};
