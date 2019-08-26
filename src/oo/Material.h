#pragma once

#include "math/Hit.h"
#include "math/Ray.h"
#include "util/Material.h"

namespace oo {
class Material {
  ::Material mat_;

public:
  Material() = default;
  Material(const ::Material &mat) : mat_(mat) {}

  // TODO handle tinting?
  struct Bounce {
    Vec3 colour;
    Ray bounced;
  };
  [[nodiscard]] Bounce bounce(const Hit &hit, const Ray &incoming, double u,
                              double v, double p) const;

  [[nodiscard]] Vec3 previewColour() const noexcept { return mat_.diffuse; }

  [[nodiscard]] Vec3 totalEmission(const Vec3 &inbound) const noexcept;
};

}
