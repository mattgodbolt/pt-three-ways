#pragma once

#include "Scene.h"
#include "math/Camera.h"
#include "util/ArrayOutput.h"

#include <functional>
#include <random>
#include <util/RenderParams.h>

namespace oo {

class Renderer {
  const Scene &scene_;
  const Camera &camera_;
  const RenderParams &renderParams_;
  ArrayOutput &output_;

  static constexpr auto FirstBounceNumUSamples = 6;
  static constexpr auto FirstBounceNumVSamples = 3;

public:
  Renderer(const Scene &scene, const Camera &camera,
           const RenderParams &renderParams, ArrayOutput &arrayOutput)
      : scene_(scene), camera_(camera), renderParams_(renderParams),
        output_(arrayOutput) {}

  void render(std::function<void()> updateFunc) const;

  // Visible for testing
  struct Tile {
    int xBegin;
    int xEnd;
    int yBegin;
    int yEnd;
    int samples;
    int sampleNum;
    size_t distancePrio;
    size_t randomPrio;
    [[nodiscard]] constexpr auto key() const {
      return std::tie(sampleNum, distancePrio, randomPrio);
    }
  };

  Vec3 radiance(std::mt19937 &rng, const Ray &ray, int depth, int numUSamples,
                int numVSamples) const;
  [[nodiscard]] std::vector<Renderer::Tile>
  generateTiles(int xTileSize, int yTileSize, int numSamples,
                int samplesPerTile) const;
  [[nodiscard]] static std::vector<Tile>
  generateTiles(int width, int height, int xTileSize, int yTileSize,
                int numSamples, int samplesPerTile);
};

}