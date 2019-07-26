#pragma once

#include "Material.h"
#include <math/Triangle.h>

#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

struct ObjFile {
  std::vector<Triangle> triangles;
  std::vector<Material> materials;
};

ObjFile loadObjFile(std::istream &in);

std::unordered_map<std::string, Material> loadMaterials(std::istream &in);
