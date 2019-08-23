#include "Renderer.h"
#include "math/Samples.h"
#include "util/WorkQueue.h"

#include <thread>

using oo::Renderer;

std::vector<Renderer::Tile> Renderer::generateTiles(int xTileSize,
                                                    int yTileSize,
                                                    int numSamples,
                                                    int samplesPerTile) const {
  return Renderer::generateTiles(renderParams_.width, renderParams_.height,
                                 xTileSize, yTileSize, numSamples,
                                 samplesPerTile);
}

std::vector<Renderer::Tile>
Renderer::generateTiles(int width, int height, int xTileSize, int yTileSize,
                        int numSamples, int samplesPerTile) {
  std::mt19937 rng(width * height);
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

namespace {}

static constexpr auto MaxDepth = 5;

Vec3 Renderer::radiance(std::mt19937 &rng, const Ray &ray, int depth,
                        int numUSamples, int numVSamples) const {
  if (depth >= MaxDepth)
    return Vec3();
  Primitive::IntersectionRecord intersectionRecord;
  if (!scene_.intersect(ray, intersectionRecord))
    return scene_.environment(ray);

  const auto &material = intersectionRecord.material;
  if (renderParams_.preview)
    return material.diffuse;
  const auto &hit = intersectionRecord.hit;

  Vec3 result;

  // Sample evenly with random offset.
  std::uniform_real_distribution<> unit(0, 1.0);
  for (auto uSample = 0; uSample < numUSamples; ++uSample) {
    for (auto vSample = 0; vSample < numVSamples; ++vSample) {
      auto u = (uSample + unit(rng)) / numUSamples;
      auto v = (vSample + unit(rng)) / numVSamples;
      auto p = unit(rng);

      auto newRay = bounce(material, hit, ray, u, v, p);
      result += radiance(rng, newRay, depth + 1, 1, 1);
    }
  }
  return material.emission
         + material.diffuse * result / (numUSamples * numVSamples);
}

// TODO: OO-ify more. Maybe hold rng as member variable, and use that as a
// non-OO Type thing? "render context" ? maybe?
// TODO: non-threaded version?
ArrayOutput
Renderer::render(std::function<void(const ArrayOutput &)> updateFunc) const {
  ArrayOutput output(renderParams_.width, renderParams_.height);

  auto renderPixel = [this](std::mt19937 &rng, int pixelX, int pixelY,
                            int numSamples) {
    Vec3 colour;
    for (int sample = 0; sample < numSamples; ++sample) {
      auto ray = camera_.randomRay(pixelX, pixelY, rng);
      colour +=
          radiance(rng, ray, 0, FirstBounceNumUSamples, FirstBounceNumVSamples);
    }
    return colour;
  };

  WorkQueue<Tile> queue(
      generateTiles(16, 16, renderParams_.samplesPerPixel, 8));

  auto worker = [&] {
    for (;;) {
      auto tileOpt = queue.pop([&] { updateFunc(output); });
      if (!tileOpt)
        break;
      auto &tile = *tileOpt;

      std::mt19937 rng(tile.randomPrio);
      for (int y = tile.yBegin; y < tile.yEnd; ++y) {
        for (int x = tile.xBegin; x < tile.xEnd; ++x) {
          output.addSamples(x, y, renderPixel(rng, x, y, tile.samples),
                            tile.samples);
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

namespace {

// TODO extract? Understand! Rephrase, use in other renderers?
double reflectance(const Norm3 &normal, const Norm3 &incoming, double iorFrom,
                   double iorTo) {
  auto iorRatio = iorFrom / iorTo;
  auto cosI = -normal.dot(incoming);
  auto sinTsquared = iorRatio * iorRatio * (1 - cosI * cosI);
  if (sinTsquared > 1) {
    // Total internal reflection.
    return 1.0;
  }
  auto cosT = sqrt(1 - sinTsquared);
  auto rOrth =
      (iorFrom * cosI - iorTo * cosT) / (iorFrom * cosI + iorTo * cosT);
  auto rPar = (iorFrom * cosI - iorTo * cosT) / (iorFrom * cosI + iorTo * cosT);
  return (rOrth * rOrth + rPar * rPar) / 2;
}

}

Ray Renderer::bounce(const Material &mat, const Hit &hit, const Ray &incoming,
                     double u, double v, double p) const {
  double iorFrom = 1.0;
  double iorTo = mat.indexOfRefraction;
  auto reflectivity = mat.reflectivity;
  if (hit.inside) {
    std::swap(iorFrom, iorTo);
  }
  if (reflectivity < 0) {
    reflectivity =
        reflectance(hit.normal, incoming.direction(), iorFrom, iorTo);
  }
  if (p < reflectivity) {
    return Ray(hit.position,
               coneSample(hit.normal.reflect(incoming.direction()),
                          mat.reflectionConeAngle(), u, v));
  } else {
    auto basis = OrthoNormalBasis::fromZ(hit.normal);
    return Ray(hit.position, hemisphereSample(basis, u, v));
  }
}
