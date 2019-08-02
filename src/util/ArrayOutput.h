#pragma once

#include "SampledPixel.h"

#include <array>
#include <vector>

class ArrayOutput {
  int width_;
  int height_;
  std::vector<SampledPixel> output_;

public:
  ArrayOutput(int width, int height) : width_(width), height_(height) {
    output_.resize(width * height);
  }
  [[nodiscard]] constexpr int height() const noexcept { return height_; }
  [[nodiscard]] constexpr int width() const noexcept { return width_; }
  void plot(int x, int y, const Vec3 &colour, int numSamples) noexcept {
    output_[x + y * width_].accumulate(colour, numSamples);
  }
  [[nodiscard]] Vec3 rawPixelAt(int x, int y) const noexcept {
    return output_[x + y * width_].result();
  }
  using Pixel = std::array<uint8_t, 3>;
  [[nodiscard]] Pixel pixelAt(int x, int y) const noexcept;
};
