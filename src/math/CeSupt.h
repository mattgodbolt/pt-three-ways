#pragma once

#include <cmath>

namespace ce_supt {

static constexpr bool isConstantEvaluated() { return false; }

constexpr double sqrt(double value) {
  return isConstantEvaluated() ? 0 : ::sqrt(value);
}

}