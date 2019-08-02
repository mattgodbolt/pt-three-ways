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
