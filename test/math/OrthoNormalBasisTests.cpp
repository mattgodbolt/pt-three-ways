#include <catch2/catch.hpp>

#include "math/OrthoNormalBasis.h"

#include <cmath>

namespace {

TEST_CASE("OrthoNormalBasis", "[OrthoNormalBasis]") {
  SECTION("constructs") {
    OrthoNormalBasis onb(Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1));
    CHECK(onb.x() == Vec3(1, 0, 0));
    CHECK(onb.y() == Vec3(0, 1, 0));
    CHECK(onb.z() == Vec3(0, 0, 1));
  }
  SECTION("creates sane ONBs") {
    auto check = [](const char *name, const OrthoNormalBasis &onb) {
      INFO(name);
      CHECK(onb.x() == Vec3(1, 0, 0));
      CHECK(onb.y() == Vec3(0, 1, 0));
      CHECK(onb.z() == Vec3(0, 0, 1));
    };
    check("xy", OrthoNormalBasis::fromXY(Vec3(1, 0, 0), Vec3(0, 1, 0)));
    check("yx", OrthoNormalBasis::fromYX(Vec3(0, 1, 0), Vec3(1, 0, 0)));
    check("xz", OrthoNormalBasis::fromXZ(Vec3(1, 0, 0), Vec3(0, 0, 1)));
    check("zx", OrthoNormalBasis::fromZX(Vec3(0, 0, 1), Vec3(1, 0, 0)));
    check("yz", OrthoNormalBasis::fromYZ(Vec3(0, 1, 0), Vec3(0, 0, 1)));
    check("zy", OrthoNormalBasis::fromZY(Vec3(0, 0, 1), Vec3(0, 1, 0)));
  }
}

}