#pragma once

#include "Sphere.h"
#include "Triangle.h"
#include "util/MaterialSpec.h"

#include <variant>

namespace fp {

struct TrianglePrimitive {
  Triangle triangle;
  MaterialSpec material;
};

struct SpherePrimitive {
  Sphere sphere;
  MaterialSpec material;
};

using Primitive = std::variant<TrianglePrimitive, SpherePrimitive>;

}