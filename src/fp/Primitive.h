#pragma once

#include "Sphere.h"
#include "Triangle.h"
#include "util/Material.h"

#include <variant>

namespace fp {

struct TrianglePrimitive {
  Triangle triangle;
  Material material;
};

struct SpherePrimitive {
  Sphere sphere;
  Material material;
};

using Primitive = std::variant<TrianglePrimitive, SpherePrimitive>;

}