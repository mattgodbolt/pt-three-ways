#include "PngWriter.h"

#include "fp/Render.h"
#include "fp/SceneBuilder.h"
#include "math/Camera.h"
#include "math/Vec3.h"
#include "oo/Renderer.h"
#include "oo/SceneBuilder.h"
#include "util/ArrayOutput.h"
#include "util/ObjLoader.h"

#include <clara.hpp>
#include <date/chrono_io.h>

#include <algorithm>
#include <chrono>
#include <dod/Scene.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <util/RenderParams.h>
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
Camera createCornellScene(SB &sb, const RenderParams &renderParams) {
  DirRelativeOpener opener("scenes");
  auto in = opener.open("CornellBox-Original.obj");
  loadObjFile(*in, opener, sb);
  sb.addSphere(Vec3(-0.38, 0.281, 0.38), 0.28,
               Material::makeReflective(Vec3(0.999, 0.999, 0.999), 0.75));
  sb.setEnvironmentColour(Vec3(0.725, 0.71, 0.68) * 0.1);
  Vec3 camPos(0, 1, 3);
  Vec3 camUp(0, 1, 0);
  Vec3 camLookAt(0, 1, 0);
  double verticalFov = 50.0;
  Camera camera(camPos, camLookAt, camUp, renderParams.width,
                renderParams.height, verticalFov);
  camera.setFocus(Vec3(0, 0, 0), 0.01);
  return camera;
}

template <typename SB>
auto createSuzanneScene(SB &sb, const RenderParams &renderParams) {
  DirRelativeOpener opener("scenes");
  auto in = opener.open("suzanne.obj");
  loadObjFile(*in, opener, sb);

  auto lightMat = Material::makeLight(Vec3(4, 4, 4));
  sb.addSphere(Vec3(0.5, 1, 3), 1, lightMat);
  sb.addSphere(Vec3(1, 1, 3), 1, lightMat);

  auto boxMat = Material::makeDiffuse(Vec3(0.20, 0.30, 0.36));
  auto tl = Vec3(-5, -5, -1);
  auto tr = Vec3(5, -5, -1);
  auto bl = Vec3(-5, 5, -1);
  auto br = Vec3(5, 5, -1);
  sb.addTriangle(tl, tr, bl, boxMat);
  sb.addTriangle(tr, bl, br, boxMat);

  Vec3 camPos(1, -0.45, 4);
  Vec3 camLookAt(1, -0.6, 0.4);
  Vec3 camUp(0, 1, 0);
  double verticalFov = 40.0;
  Camera camera(camPos, camLookAt, camUp, renderParams.width,
                renderParams.height, verticalFov);
  camera.setFocus(camLookAt, 0.01);
  return camera;
}

template <typename SB>
auto createCeScene(SB &sb, const RenderParams &renderParams) {
  DirRelativeOpener opener("scenes");
  auto in = opener.open("ce.obj");
  loadObjFile(*in, opener, sb);

  auto brightLight = Material::makeLight(Vec3(40, 40, 40));
  sb.addSphere(Vec3(0, 1.6, 0), 0.15, brightLight);
  auto dullLight = Material::makeLight(Vec3(2.27, 3, 2.97));
  sb.addSphere(Vec3(-0.2, 5.9, -0.3), 1.0, dullLight);

  auto boxMat = Material::makeDiffuse(Vec3(0.20, 0.30, 0.36));
  auto tl = Vec3(-5, -1, -5);
  auto tr = Vec3(5, -1, -5);
  auto bl = Vec3(-5, 1, -5);
  auto br = Vec3(5, 1, -5);
  sb.addTriangle(tl, tr, bl, boxMat);
  sb.addTriangle(tr, bl, br, boxMat);

  sb.addSphere(Vec3(), 10, Material::makeDiffuse(Vec3(0.2, 0.2, 0.2)));

  Vec3 camPos(0.27, 1.25, 0.36);
  Vec3 camLookAt(0, 0, 0);
  Vec3 camUp(0, 0, -1);
  double verticalFov = 40.0;
  Camera camera(camPos, camLookAt, camUp, renderParams.width,
                renderParams.height, verticalFov);
  camera.setFocus(camLookAt, 0.01);
  return camera;
}

template <typename SB>
auto createScene(SB &sb, const std::string &sceneName,
                 const RenderParams &renderParams) {
  if (sceneName == "cornell")
    return createCornellScene(sb, renderParams);
  if (sceneName == "suzanne")
    return createSuzanneScene(sb, renderParams);
  if (sceneName == "ce")
    return createCeScene(sb, renderParams);
  throw std::runtime_error("Unknown scene " + sceneName);
}

}

int main(int argc, const char *argv[]) {
  using namespace clara;
  using namespace std::literals;

  bool help = false;
  RenderParams renderParams;
  std::string way = "oo";
  std::string sceneName = "cornell";
  std::string outputName;

  auto cli =
      Opt(renderParams.width, "width")["-w"]["--width"]("output image width")
      | Opt(renderParams.height,
            "height")["-h"]["--height"]("output image height")
      | Opt(renderParams.maxCpus, "numCpus")["--num-cpus"](
          "maximum number of CPUs to use (0 for all)")
      | Opt(renderParams.samplesPerPixel,
            "samples")["--spp"]("number of samples per pixel")
      | Opt(renderParams.preview)["--preview"]("super quick preview")
      | Opt(way, "way")["--way"]("which way, oo (the default), fp or dod")
      | Opt(sceneName, "scene")["--scene"]("which scene to render")
      | Arg(outputName, "output")("output filename").required() | Help(help);

  auto result = cli.parse(Args(argc, argv));
  if (!result) {
    std::cerr << "Error in command line: " << result.errorMessage() << '\n';
    exit(1);
  }
  if (help) {
    std::cout << cli;
    exit(0);
  }

  if (outputName.empty()) { // https://github.com/catchorg/Clara/issues/39
    std::cerr << "Missing output filename.\n" << cli;
    exit(1);
  }

  if (renderParams.maxCpus == 0) {
    renderParams.maxCpus =
        static_cast<int>(std::thread::hardware_concurrency());
  }

  auto startTime = std::chrono::system_clock::now();
  ArrayOutput output(renderParams.width, renderParams.height);

  auto save = [&]() {
    PngWriter pw(outputName.c_str(), renderParams.width, renderParams.height);
    if (!pw.ok()) {
      std::cerr << "Unable to save PNG\n";
      return;
    }

    for (int y = 0; y < renderParams.height; ++y) {
      std::uint8_t row[renderParams.width * 3];
      for (int x = 0; x < renderParams.width; ++x) {
        auto colour = output.pixelAt(x, y);
        for (int component = 0; component < 3; ++component)
          row[x * 3 + component] = colour[component];
      }
      pw.addRow(row);
    }
  };

  if (way == "oo") {
    static constexpr auto saveEvery = 10s;
    auto nextSave = std::chrono::system_clock::now() + saveEvery;
    oo::SceneBuilder sceneBuilder;
    auto camera = createScene(sceneBuilder, sceneName, renderParams);
    oo::Renderer renderer(sceneBuilder.scene(), camera, renderParams, output);
    renderer.render([&] {
      // TODO: save is not thread safe even slightly, and yet it still blocks
      // the threads. this is terrible. Should have a thread safe result queue
      // and a single thread reading from it.
      auto now = std::chrono::system_clock::now();
      if (now > nextSave) {
        save();
        nextSave = now + saveEvery;
      }
    });
  } else if (way == "fp") {
    fp::SceneBuilder sceneBuilder;
    auto camera = createScene(sceneBuilder, sceneName, renderParams);
    fp::render(camera, sceneBuilder.scene(), renderParams, output, save);
  } else if (way == "dod") {
    dod::Scene scene;
    auto camera = createScene(scene, sceneName, renderParams);
    scene.render(camera, renderParams, output, save);
  } else {
    std::cerr << "Unknown way " << way << '\n';
    return 1;
  }

  save();

  using namespace date;
  auto endTime = std::chrono::system_clock::now();
  auto timeTaken = endTime - startTime;
  auto totalSamples = output.totalSamples();
  std::cout << "Took "
            << std::chrono::duration_cast<std::chrono::seconds>(timeTaken)
            << "\n";
  std::cout << "Total samples: " << totalSamples << "\n";
  auto samplesPerSec =
      static_cast<double>(totalSamples)
      / std::chrono::duration_cast<std::chrono::milliseconds>(timeTaken)
            .count();
  std::cout << "Samples/ms: " << samplesPerSec << "\n";
}