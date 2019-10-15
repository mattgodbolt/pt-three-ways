#include <catch2/catch.hpp>

#include <cstdio>
#ifdef __GNUC__
#if __GNUC__ < 8
#include <experimental/filesystem>
using std_filesystem = std::experimental::filesystem;
#else
#include <filesystem>
using std_filesystem = std::filesystem;
#endif
#else
#include <filesystem>
using std_filesystem = std::filesystem;
#endif

#include <string>
#include <random>

#include "util/ArrayOutput.h"

TEST_CASE("ArrayOutput", "[ArrayOutput]") {
  SECTION("Constructs sensibly") {
    ArrayOutput ao(10, 20);
    CHECK(ao.width() == 10);
    CHECK(ao.height() == 20);
    CHECK(ao.pixelAt(0, 0) == ArrayOutput::Pixel{0, 0, 0});
    CHECK(ao.rawPixelAt(0, 0) == Vec3());
  }
  SECTION("Roundtrips through a file") {
    const std::string tempBuf = []() {
      std::string tempBuf =
          (std_filesystem::temp_directory_path() / "arrayoutputtest")
              .string();
      FILE *f = fopen(tempBuf.c_str(), "wx");
      if (f == nullptr) {
        std::random_device random_device;
        std::mt19937 random_engine(random_device());
        std::uniform_int_distribution<char> distribution(0, 35);
        do {
          const char value = distribution(random_engine);
          tempBuf.push_back(value < 10 ? '0' + value : 'a' + static_cast<char>(value - 10));
          f = fopen(tempBuf.c_str(), "wx");
        } while (f == nullptr);
      }
      fclose(f);
      return tempBuf;
    }();
    ArrayOutput ao(7, 5);
    ao.addSamples(0, 0, Vec3(0.2, 0.3, 0.4), 12);
    ao.addSamples(1, 0, Vec3(0.4, 0.6, 0.7), 1);
    ao.addSamples(0, 3, Vec3(0.1, 0.2, 0.3), 2);
    ao.save(tempBuf);
    auto loaded = ArrayOutput::load(tempBuf);
    std::remove(tempBuf.c_str()); // TODO RAIIfy

    REQUIRE(loaded.width() == ao.width());
    REQUIRE(loaded.height() == ao.height());
    for (int y = 0; y < loaded.height(); ++y) {
      INFO("y=" << y);
      for (int x = 0; x < loaded.width(); ++x) {
        INFO("x=" << x);
        REQUIRE(loaded.rawPixelAt(x, y) == ao.rawPixelAt(x, y));
        REQUIRE(loaded.pixelAt(x, y) == ao.pixelAt(x, y));
        REQUIRE(loaded.pixelAt(x, y) == ao.pixelAt(x, y));
      }
    }
  }
}
