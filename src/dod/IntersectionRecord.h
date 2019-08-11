#pragma once

#include "math/Hit.h"
#include "util/Material.h"

namespace dod {

struct IntersectionRecord {
  Hit hit;
  Material material;
};

}