#pragma once

#include <random>
#include <utility>

namespace fp {

template <typename Rng>
class Random2DSampler {
  Rng &rng_;
  int numUSamples_;
  int numVSamples_;

public:
  Random2DSampler(Rng &rng, int numUSamples, int numVSamples)
      : rng_(rng), numUSamples_(numUSamples), numVSamples_(numVSamples) {}

  class Iterator {
    const Random2DSampler &sampler_;
    int u_{};
    int v_{};

  public:
    Iterator(const Random2DSampler &sampler, int u, int v)
        : sampler_(sampler), u_(u), v_(v) {}
    std::pair<double, double> operator*() const {
      std::uniform_real_distribution<> unit(0, 1.0);
      const auto sampleU = (static_cast<double>(u_) + unit(sampler_.rng_))
                           / static_cast<double>(sampler_.numUSamples_);
      const auto sampleV = (static_cast<double>(v_) + unit(sampler_.rng_))
                           / static_cast<double>(sampler_.numVSamples_);
      return std::make_pair(sampleU, sampleV);
    }
    Iterator &operator++() {
      if (++u_ == sampler_.numUSamples_) {
        u_ = 0;
        v_++;
      }
      return *this;
    }
    bool operator==(const Iterator &rhs) const {
      return u_ == rhs.u_ && v_ == rhs.v_;
    }
    bool operator!=(const Iterator &rhs) const { return !(rhs == *this); }
  };

  [[nodiscard]] Iterator begin() const { return Iterator(*this, 0, 0); }
  [[nodiscard]] Iterator end() const {
    return Iterator(*this, 0, numVSamples_);
  }
};

}