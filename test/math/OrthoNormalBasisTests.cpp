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
  SECTION("creates ONBs for single axes") {
    auto checkIsBasis = [](const char *name, const OrthoNormalBasis &onb) {
      INFO(name);
      CHECK(onb.x().lengthSquared() == Approx(1.));
      CHECK(onb.y().lengthSquared() == Approx(1.));
      CHECK(onb.z().lengthSquared() == Approx(1.));
      auto basicallyZero =Approx(0.).margin(0.00000001);
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