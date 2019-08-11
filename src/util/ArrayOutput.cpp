#include "ArrayOutput.h"

#include <algorithm>

namespace {
std::uint8_t componentToInt(double x) {
  return static_cast<uint8_t>(
      lround(pow(std::clamp(x, 0.0, 1.0), 1.0 / 2.2) * 255));
}
}

ArrayOutput::Pixel ArrayOutput::pixelAt(int x, int y) const noexcept {
  auto rawPixel = rawPixelAt(x, y);
  return ArrayOutput::Pixel{componentToInt(rawPixel.x()),
                            componentToInt(rawPixel.y()),
                            componentToInt(rawPixel.z())};
}

void ArrayOutput::addSamples(int x, int y, const Vec3 &colour,
                             int numSamples) noexcept {
  output_[indexOf(x, y)].accumulate(colour, numSamples);
}

Vec3 ArrayOutput::rawPixelAt(int x, int y) const noexcept {
  return output_[indexOf(x, y)].result();
}

ArrayOutput &ArrayOutput::operator+=(const ArrayOutput &rhs) {
  if (rhs.width() != width() || rhs.height() != height())
    throw std::logic_error(
        "Two differently-sized arrays were attempted to be combined");
  for (size_t pixelIndex = 0; pixelIndex < output_.size(); ++pixelIndex) {
    output_[pixelIndex].accumulate(rhs.output_[pixelIndex]);
  }
  return *this;
}
