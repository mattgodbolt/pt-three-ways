#include "Renderer.h"
#include "util/WorkQueue.h"

#include <thread>
#include <util/Metric.h>

using oo::Renderer;

std::vector<Renderer::Tile>
Renderer::generateTiles(int xTileSize, int yTileSize, int numSamples,
                        int samplesPerTile, int seed) const {
  return Renderer::generateTiles(renderParams_.width, renderParams_.height,
                                 xTileSize, yTileSize, numSamples,
                                 samplesPerTile, seed);
}

std::vector<Renderer::Tile>
Renderer::generateTiles(int width, int height, int xTileSize, int yTileSize,
                        int numSamples, int samplesPerTile, int seed) {
  std::mt19937 rng(seed);
  std::vector<Tile> tiles;
  for (int y = 0; y < height; y += yTileSize) {
    int yBegin = y;
    int yEnd = std::min(y + yTileSize, height);
    for (int x = 0; x < width; x += xTileSize) {
      int xBegin = x;
      int xEnd = std::min(x + xTileSize, width);
      int midX = (xEnd + xBegin) / 2;
      int midY = (yEnd + yBegin) / 2;
      int centreX = width / 2;
      int centreY = height / 2;
      size_t distanceSqr = (midX - centreX) * (midX - centreX)
                           + (midY - centreY) * (midY - centreY);
      for (int s = 0; s < numSamples; s += samplesPerTile) {
        int nSamples = std::min(s + samplesPerTile, numSamples) - s;
        tiles.emplace_back(
            Tile{xBegin, xEnd, yBegin, yEnd, nSamples, s, distanceSqr, rng()});
      }
    }
  }
  std::sort(tiles.begin(), tiles.end(), [](const Tile &lhs, const Tile &rhs) {
    return lhs.key() > rhs.key();
  });
  return tiles;
}

Vec3 Renderer::radiance(std::mt19937 &rng, const Ray &ray, int depth,
                        ThreadMetrics &metrics) const {
  metrics.radianceCalled.increment();
  if (depth >= renderParams_.maxDepth)
    return Vec3();
  int numUSamples = depth == 0 ? renderParams_.firstBounceUSamples : 1;
  int numVSamples = depth == 0 ? renderParams_.firstBounceVSamples : 1;
  Primitive::IntersectionRecord intersectionRecord;
  if (!scene_.intersect(ray, intersectionRecord))
    return scene_.environment(ray);

  const auto &material = intersectionRecord.material;
  if (renderParams_.preview)
    return material.previewColour();
  const auto &hit = intersectionRecord.hit;

  Vec3 result;

  // Sample evenly with random offset.
  std::uniform_real_distribution<> unit(0, 1.0);
  for (auto uSample = 0; uSample < numUSamples; ++uSample) {
    for (auto vSample = 0; vSample < numVSamples; ++vSample) {
      auto u = (uSample + unit(rng)) / numUSamples;
      auto v = (vSample + unit(rng)) / numVSamples;
      auto p = unit(rng);

      auto nextPath = material.bounce(hit, ray, u, v, p);
      result += nextPath.colour * radiance(rng, nextPath.bounced, depth + 1);
    }
  }
  return material.totalEmission(result / (numUSamples * numVSamples));
}

// TODO: OO-ify more. Maybe hold rng as member variable, and use that as a
// non-OO Type thing? "render context" ? maybe?
// TODO: non-threaded version?
ArrayOutput
Renderer::render(std::function<void(const ArrayOutput &)> updateFunc) const {
  ArrayOutput output(renderParams_.width, renderParams_.height);

  auto renderPixel = [this](std::mt19937 &rng, int pixelX, int pixelY,
                            int numSamples, ThreadMetrics &metrics) {
    Vec3 colour;
    metrics.pixelsRendered.increment();
    for (int sample = 0; sample < numSamples; ++sample) {
      metrics.samplesRendered.increment();
      auto ray = camera_.randomRay(pixelX, pixelY, rng);
      colour += radiance(rng, ray, 0, metrics);
    }
    return colour;
  };

  WorkQueue<Tile> queue(generateTiles(16, 16, renderParams_.samplesPerPixel, 8,
                                      renderParams_.seed));

  auto worker = [&] {
    ThreadMetrics metrics;
    for (;;) {
      auto tileOpt = queue.pop([&] { updateFunc(output); });
      if (!tileOpt)
        break;
      auto &tile = *tileOpt;

      std::mt19937 rng(tile.randomPrio);
      for (int y = tile.yBegin; y < tile.yEnd; ++y) {
        for (int x = tile.xBegin; x < tile.xEnd; ++x) {
          output.addSamples(x, y, renderPixel(rng, x, y, tile.samples),
                            tile.samples, metrics);
        }
      }
    }
  };
  std::vector<std::thread> threads{static_cast<size_t>(renderParams_.maxCpus)};
  std::generate(threads.begin(), threads.end(),
                [&]() { return std::thread(worker); });
  for (auto &t : threads)
    t.join();

  return output;
}
