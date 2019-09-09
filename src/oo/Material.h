#pragma once

#include "math/Hit.h"
#include "math/Ray.h"
#include "util/Material.h"

namespace oo {

class Renderer;

class Material {
  ::Material mat_;

public:
  Material() = default;
  Material(const ::Material &mat) : mat_(mat) {}

  class RadianceSampler {
  public:
    virtual ~RadianceSampler() = default;

    virtual Vec3 sample(const Ray &ray) const = 0;
  };

  [[nodiscard]] Vec3 sample(const Hit &hit, const Ray &incoming,
                            const RadianceSampler &radianceSampler, double u,
                            double v, double p) const;

  [[nodiscard]] Vec3 previewColour() const noexcept { return mat_.diffuse; }

  [[nodiscard]] Vec3 totalEmission(const Vec3 &inbound) const noexcept;
};

}
