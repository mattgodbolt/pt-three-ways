#pragma once

#include "oo/Primitive.h"

#include <memory>
#include <vector>

namespace oo {

class Scene : public Primitive {
  std::vector<std::unique_ptr<Primitive>> primitives_;

public:
  void add(std::unique_ptr<Primitive> primitive);

  [[nodiscard]] bool intersect(const Ray &ray,
                               IntersectionRecord &intersection) const override;
};

}