#include <catch2/catch.hpp>

#include "fp/Random2DSampler.h"

#include <cmath>

using fp::Random2DSampler;

namespace {

double sampleNth(size_t seed, size_t nth) {
  std::mt19937 rng(seed);
  std::uniform_real_distribution dist;
  for (size_t i = 0; i < nth; ++i)
    dist(rng);
  return dist(rng);
}

TEST_CASE("Random2DSampler", "[Random2DSampler]") {
  constexpr auto seed = 1234;

  SECTION("works for a single sample") {
    std::mt19937 rng(seed);
    Random2DSampler sampler(rng, 1, 1);
    auto it = std::begin(sampler);
    CHECK(it != std::end(sampler));
    auto res = *it;
    CHECK(res.first == sampleNth(seed, 0));
    CHECK(res.second == sampleNth(seed, 1));
    ++it;
    CHECK(it == std::end(sampler));
  }

  SECTION("works for 2x2 samples") {
    std::mt19937 rng(seed);
    Random2DSampler sampler(rng, 2, 2);
    auto it = std::begin(sampler);
    CHECK(it != std::end(sampler));
    auto res = *it;
    CHECK(res.first == sampleNth(seed, 0) / 2);
    CHECK(res.second == sampleNth(seed, 1) / 2);
    ++it;
    CHECK(it != std::end(sampler));
    res = *it;
    CHECK(res.first == sampleNth(seed, 2) / 2 + 0.5);
    CHECK(res.second == sampleNth(seed, 3) / 2);
    ++it;
    CHECK(it != std::end(sampler));
    res = *it;
    CHECK(res.first == sampleNth(seed, 4) / 2);
    CHECK(res.second == sampleNth(seed, 5) / 2 + 0.5);
    ++it;
    CHECK(it != std::end(sampler));
    res = *it;
    CHECK(res.first == sampleNth(seed, 6) / 2 + 0.5);
    CHECK(res.second == sampleNth(seed, 7) / 2 + 0.5);
    ++it;
    CHECK(it == std::end(sampler));
  }
}

}