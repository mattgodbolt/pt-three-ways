#pragma once

#include "Scene.h"

namespace oo {

class SceneBuilder {
  Scene scene_;

public:
  void addTriangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2,
                   const Material &material);
  void addSphere(const Vec3 &centre, double radius, const Material &material);

  void setEnvironmentColour(const Vec3 &colour);

  const Scene &scene() const { return scene_; }
};

}