#include <catch2/catch.hpp>

#include "math/OrthoNormalBasis.h"

#include <cmath>

namespace {

TEST_CASE("OrthoNormalBasis", "[OrthoNormalBasis]") {
  SECTION("constructs") {
    OrthoNormalBasis onb(Norm3::xAxis(), Norm3::yAxis(), Norm3::zAxis());
    CHECK(onb.x() == Norm3::xAxis());
    CHECK(onb.y() == Norm3::yAxis());
    CHECK(onb.z() == Norm3::zAxis());
  }
  SECTION("creates sane ONBs") {
    auto check = [](const char *name, const OrthoNormalBasis &onb) {
      INFO(name);
      CHECK(onb.x() == Norm3::xAxis());
      CHECK(onb.y() == Norm3::yAxis());
      CHECK(onb.z() == Norm3::zAxis());
    };
    check("xy", OrthoNormalBasis::fromXY(Vec3(1, 0, 0), Vec3(0, 1, 0)));
    check("yx", OrthoNormalBasis::fromYX(Vec3(0, 1, 0), Vec3(1, 0, 0)));
    check("xz", OrthoNormalBasis::fromXZ(Vec3(1, 0, 0), Vec3(0, 0, 1)));
    check("zx", OrthoNormalBasis::fromZX(Vec3(0, 0, 1), Vec3(1, 0, 0)));
    check("yz", OrthoNormalBasis::fromYZ(Vec3(0, 1, 0), Vec3(0, 0, 1)));
    check("zy", OrthoNormalBasis::fromZY(Vec3(0, 0, 1), Vec3(0, 1, 0)));
  }
  SECTION("creates ONBs for single axes") {
    auto checkIsBasis = [](const char *name, const OrthoNormalBasis &onb) {
      INFO(name);
      auto basicallyZero = Approx(0.).margin(0.00000001);
      CHECK(onb.x().dot(onb.y()) == basicallyZero);
      CHECK(onb.x().dot(onb.z()) == basicallyZero);
      CHECK(onb.y().dot(onb.z()) == basicallyZero);
    };
    checkIsBasis("z100", OrthoNormalBasis::fromZ(Vec3(1, 0, 0)));
    checkIsBasis("z010", OrthoNormalBasis::fromZ(Vec3(0, 1, 0)));
    checkIsBasis("z001", OrthoNormalBasis::fromZ(Vec3(0, 0, 1)));
    checkIsBasis("z002", OrthoNormalBasis::fromZ(Vec3(0, 0, 2)));
    checkIsBasis("zn00", OrthoNormalBasis::fromZ(Vec3(-1, 0, 0)));
    checkIsBasis("z0n0", OrthoNormalBasis::fromZ(Vec3(0, -1, 0)));
    checkIsBasis("z00n", OrthoNormalBasis::fromZ(Vec3(0, 0, -1)));
    checkIsBasis("zrnd",
                 OrthoNormalBasis::fromZ(Vec3(-0.211944, -0.495198, 0.842530)));
  }
}

}