#include "Progressifier.h"

#include <date/date.h>

#include <chrono>
#include <iomanip>
#include <iostream>

Progressifier::Progressifier(size_t numWork) noexcept : numWork_(numWork) {}

void Progressifier::update(size_t numDone) noexcept {
  auto progress = static_cast<double>(numWork_ - numDone) / numWork_ * 100;
  if (progress >= lastProgress_ + minProgress_) {
    auto now = std::chrono::system_clock::now();
    using namespace date;
    std::cout << now << " : " << std::fixed << std::setprecision(2) << progress
              << "%\n"
              << std::flush;
    lastProgress_ = progress;
  }
}
