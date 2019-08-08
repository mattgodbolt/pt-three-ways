#include "PngWriter.h"

#include "math/Camera.h"
#include "math/Vec3.h"
#include "oo/Renderer.h"
#include "oo/SceneBuilder.h"
#include "util/ArrayOutput.h"
#include "util/ObjLoader.h"

#include <clara.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace {

struct DirRelativeOpener : ObjLoaderOpener {
  std::string dir_;
  explicit DirRelativeOpener(std::string dir) : dir_(std::move(dir)) {}
  [[nodiscard]] std::unique_ptr<std::istream>
  open(const std::string &filename) override {
    auto fullname = dir_ + "/" + filename;
    auto res = std::make_unique<std::ifstream>(fullname);
    if (!*res)
      throw std::runtime_error("Unable to open " + fullname);
    return res;
  }
};

template <typename SB>
void createCornellScene(SB &sb) {
  DirRelativeOpener opener("scenes");
  auto in = opener.open("CornellBox-Original.obj");
  loadObjFile(*in, opener, sb);
  sb.addSphere(Vec3(-0.38, 0.281, 0.38), 0.28,
               Material::makeReflective(Vec3(0.999, 0.999, 0.999), 0.75));
  sb.setEnvironmentColour(Vec3(0.725, 0.71, 0.68) * 0.1);
}

}

int main(int argc, const char *argv[]) {
  bool help = false;
  auto width = 1920;
  auto height = 1080;
  auto numCpus = 1u;
  auto samplesPerPixel = 40;
  bool preview = false;

  using namespace clara;
  auto cli =
      Opt(width, "width")["-w"]["--width"]("output image width")
      | Opt(height, "height")["-h"]["--height"]("output image height")
      | Opt(numCpus,
            "numCpus")["--num-cpus"]("number of CPUs to use (0 for all)")
      | Opt(samplesPerPixel, "samples")["--spp"]("number of samples per pixel")
      | Opt(preview)["--preview"]("super quick preview") | Help(help);

  auto result = cli.parse(Args(argc, argv));
  if (!result) {
    std::cerr << "Error in command line: " << result.errorMessage() << '\n';
    exit(1);
  }
  if (help) {
    std::cout << cli;
    exit(0);
  }

  if (numCpus == 0) {
    numCpus = std::thread::hardware_concurrency();
  }

  ArrayOutput output(width, height);
  Vec3 camPos(0, 1, 3);
  Vec3 camUp(0, 1, 0);
  Vec3 camLookAt(0, 1, 0);
  double verticalFov = 50.0;
  Camera camera(camPos, camLookAt, camUp, width, height, verticalFov);
  camera.setFocus(Vec3(0, 0, 0), 0.01);

  auto save = [&]() {
    PngWriter pw("image.png", width, height);
    if (!pw.ok()) {
      std::cerr << "Unable to save PNG\n";
      return;
    }

    for (int y = 0; y < height; ++y) {
      std::uint8_t row[width * 3];
      for (int x = 0; x < width; ++x) {
        auto colour = output.pixelAt(x, y);
        for (int component = 0; component < 3; ++component)
          row[x * 3 + component] = colour[component];
      }
      pw.addRow(row);
    }
  };

  using namespace std::literals;
  static constexpr auto saveEvery = 10s;
  auto nextSave = std::chrono::system_clock::now() + saveEvery;
  oo::SceneBuilder sceneBuilder;
  createCornellScene(sceneBuilder);
  oo::Renderer renderer(sceneBuilder.scene(), camera, output, samplesPerPixel,
                        numCpus, preview);
  renderer.render([&] {
    // TODO: save is not thread safe even slightly, and yet it still blocks the
    // threads. this is terrible. Should have a thread safe result queue and
    // a single thread reading from it.
    auto now = std::chrono::system_clock::now();
    if (now > nextSave) {
      save();
      nextSave = now + saveEvery;
    }
  });

  save();
}