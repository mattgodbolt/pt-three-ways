#pragma once

#include "Material.h"
#include "Triangle.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace oo {

struct ObjFile {
  std::vector<Triangle> triangles;
  std::vector<Material> materials;
};

struct ObjLoaderOpener {
  virtual ~ObjLoaderOpener() = default;
  virtual std::unique_ptr<std::istream> open(const std::string &filename) = 0;
};

[[nodiscard]] ObjFile loadObjFile(std::istream &in, ObjLoaderOpener &opener);

[[nodiscard]] std::unordered_map<std::string, Material>
loadMaterials(std::istream &in);

}