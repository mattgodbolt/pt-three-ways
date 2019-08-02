#pragma once

#include <math/Vec3.h>

class SampledPixel {
  Vec3 colour_;
  size_t numSamples_{};

public:
  void accumulate(const Vec3 &sample, int num) {
    colour_ += sample;
    numSamples_ += num;
  }
  [[nodiscard]] Vec3 result() const {
    if (numSamples_ == 0)
      return colour_;
    return colour_ * (1.0 / numSamples_);
  }
};
