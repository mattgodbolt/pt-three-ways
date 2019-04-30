#include <catch2/catch.hpp>

#include "math/Vec3.h"

namespace {

TEST_CASE("Vectors", "[math]") {
  SECTION("default constructs to zero") {
    CHECK(Vec3().x() == 0);
    CHECK(Vec3().y() == 0);
    CHECK(Vec3().z() == 0);
  }
  SECTION("can be constructed") {
    auto vec = Vec3(1., 2., 3.);
    CHECK(vec.x() == 1.);
    CHECK(vec.y() == 2.);
    CHECK(vec.z() == 3.);
  }
  SECTION("comparison") {
    CHECK(Vec3() == Vec3());
    CHECK(Vec3() != Vec3(1, 2, 3));
    CHECK(Vec3(1, 2, 3) == Vec3(1, 2, 3));
    CHECK(Vec3(1, 2, 3) != Vec3(3, 2, 1));
  }
  SECTION("adding") {
    CHECK(Vec3() + Vec3() == Vec3());
    CHECK(Vec3(1, 1, 1) + Vec3(2, 2, 2) == Vec3(3, 3, 3));
    auto vec = Vec3(1, 2, 3);
    CHECK((vec += Vec3(2, 4, 6)) == Vec3(3, 6, 9));
    CHECK(vec == Vec3(3, 6, 9));
  }
}

}