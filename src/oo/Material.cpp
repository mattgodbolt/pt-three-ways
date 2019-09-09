#include "Material.h"
#include "math/OrthoNormalBasis.h"
#include "math/Samples.h"

namespace oo {

Vec3 Material::sample(const Hit &hit, const Ray &incoming,
                      const RadianceSampler &radianceSampler, double u,
                      double v, double p) const {
  double iorFrom = 1.0;
  double iorTo = mat_.indexOfRefraction;
  auto reflectivity = mat_.reflectivity;
  if (hit.inside) {
    std::swap(iorFrom, iorTo);
  }
  if (reflectivity < 0) {
    reflectivity = hit.normal.reflectance(incoming.direction(), iorFrom, iorTo);
  }
  if (p < reflectivity) {
    return radianceSampler.sample(
        Ray(hit.position, coneSample(hit.normal.reflect(incoming.direction()),
                                     mat_.reflectionConeAngleRadians, u, v)));
  } else {
    auto basis = OrthoNormalBasis::fromZ(hit.normal);
    return mat_.diffuse
           * radianceSampler.sample(
               Ray(hit.position, hemisphereSample(basis, u, v)));
  }
}

Vec3 Material::totalEmission(const Vec3 &inbound) const noexcept {
  return mat_.emission + inbound;
}

}
