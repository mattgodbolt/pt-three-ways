#pragma once

#include "SampledPixel.h"

#include <array>
#include <vector>

class ArrayOutput {
  const int width_;
  const int height_;
  std::vector<SampledPixel> output_;

  [[nodiscard]] constexpr int indexOf(int x, int y) const noexcept {
    // TODO "assert" in range?
    return x + y * width_;
  }

public:
  ArrayOutput(int width, int height) : width_(width), height_(height) {
    output_.resize(width * height);
  }
  [[nodiscard]] constexpr int height() const noexcept { return height_; }
  [[nodiscard]] constexpr int width() const noexcept { return width_; }

  void addSamples(int x, int y, const Vec3 &colour, int numSamples) noexcept;

  [[nodiscard]] Vec3 rawPixelAt(int x, int y) const noexcept;

  using Pixel = std::array<uint8_t, 3>;
  [[nodiscard]] Pixel pixelAt(int x, int y) const noexcept;

  ArrayOutput &operator+=(const ArrayOutput &rhs);

  size_t totalSamples() const noexcept;
};
