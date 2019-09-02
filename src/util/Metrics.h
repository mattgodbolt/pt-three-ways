#pragma once

struct Metrics {
  int numTriHits{};
  int numTriMisses{};
  int numSphereHits{};
  int numSphereMisses{};
  int numRadianceCalls{};

  static Metrics &the() {
    static Metrics m;
    return m;
  }
};