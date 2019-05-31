#include "Progressifier.h"

#include <cstdio>

Progressifier::Progressifier(size_t numWork) noexcept : numWork_(numWork) {}

void Progressifier::update(size_t numDone) noexcept {
  auto progress = static_cast<double>(numWork_ - numDone) / numWork_ * 100;
  if (progress >= lastProgress_ + minProgress_) {
    printf("%.2f%%\n", progress);
    lastProgress_ = progress;
  }
}
