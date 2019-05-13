#pragma once

#include "Vec3.h"

struct Hit {
  double distance{};
  Vec3 position;
  Vec3 normal;
};
