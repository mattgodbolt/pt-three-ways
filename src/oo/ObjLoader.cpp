#include "ObjLoader.h"

#include <fstream>
#include <string>

#include <ctre.hpp>

using namespace std::literals;

static constexpr auto tokenRe = ctll::fixed_string{R"(\s*([^ \t\n\r]+))"};

namespace {

double asDouble(std::string_view sv) {
  return std::stod(std::string(sv)); // This is dreadful
}

size_t asIndex(std::string_view sv, size_t max) {
  auto res = std::stol(std::string(sv));
  return res < 0 ? res + max : res;
}

template <typename F>
void parse(std::istream &in, F &&handler) {
  std::string lineAsString;
  int lineNumber = 0;
  while (std::getline(in, lineAsString)) {
    lineNumber++;
    std::string_view line(lineAsString);

    std::vector<std::string_view> fields;
    for (auto match : ctre::range<tokenRe>(line)) {
      if (!match)
        break;
      fields.emplace_back(match.get<1>().to_view());
    }

    if (fields.empty() || fields[0][0] == '#')
      continue;

    auto command = fields.front();
    fields.erase(fields.begin());
    if (!handler(command, fields)) {
      throw std::runtime_error("Unknown directive '" + std::string(command)
                               + "' on line " + std::to_string(lineNumber));
    }
  }
}

}

std::unordered_map<std::string, Material> loadMaterials(std::istream &in) {
  if (!in)
    throw std::runtime_error("Bad input stream");
  in.exceptions(std::ios_base::badbit);
  std::unordered_map<std::string, Material> result;

  Material *curMat{};

  parse(in, [&](std::string_view command,
                const std::vector<std::string_view> &params) {
    if (command == "newmtl"sv) {
      if (params.size() != 1)
        throw std::runtime_error("Wrong number of params for newmtl");
      curMat =
          &result.emplace(std::string(params[0]), Material{}).first->second;
      return true;
    } else if (command == "Ke"sv) {
      if (!curMat)
        throw std::runtime_error("Unexpected Ke");
      if (params.size() != 3)
        throw std::runtime_error("Wrong number of params for Ke");
      curMat->emission =
          Vec3(asDouble(params[0]), asDouble(params[1]), asDouble(params[2]));
      return true;
    } else if (command == "Kd"sv) {
      if (!curMat)
        throw std::runtime_error("Unexpected Kd");
      if (params.size() != 3)
        throw std::runtime_error("Wrong number of params for Kd");
      curMat->diffuse =
          Vec3(asDouble(params[0]), asDouble(params[1]), asDouble(params[2]));
      return true;
    } else if (command == "Ns"sv || command == "Ni"sv || command == "illum"sv
               || command == "Ka"sv || command == "Ks"sv) {
      // Ignored
      return true;
    }
    return false;
  });

  return result;
}

// Thanks to https://en.wikipedia.org/wiki/Wavefront_.obj_file
ObjFile loadObjFile(std::istream &in, ObjLoaderOpener &opener) {
  if (!in)
    throw std::runtime_error("Bad input stream");
  in.exceptions(std::ios_base::badbit);

  ObjFile result;

  std::vector<Vec3> vertices;
  std::unordered_map<std::string, Material> materials;
  Material curMat;

  parse(in, [&](std::string_view command,
                const std::vector<std::string_view> &params) {
    if (command == "v"sv) {
      if (params.size() != 3)
        throw std::runtime_error("Wrong number of params for v");
      vertices.emplace_back(asDouble(params[0]), asDouble(params[1]),
                            asDouble(params[2]));
      return true;
    } else if (command == "f"sv) {
      // Decimate the face as a fan.
      std::vector<size_t> indices;
      indices.reserve(params.size());
      for (auto f : params)
        indices.emplace_back(asIndex(f, vertices.size()));
      for (size_t index = 1; index < params.size() - 1; ++index) {
        result.triangles.emplace_back(vertices.at(indices[0]),
                                      vertices.at(indices[index]),
                                      vertices.at(indices[index + 1]));
        result.materials.emplace_back(curMat);
      }
      return true;
    } else if (command == "g"sv) {
      // Ignore groups
      return true;
    } else if (command == "usemtl"sv) {
      auto matName = std::string(params.at(0));
      auto findIt = materials.find(matName);
      if (findIt == materials.end())
        throw std::runtime_error("Can't find material " + matName);
      curMat = findIt->second;
      return true;
    } else if (command == "mtllib"sv) {
      auto matFile = opener.open(std::string(params.at(0)));
      materials = loadMaterials(*matFile);
      return true;
    }
    return false;
  });
  return result;
}