#include "ObjLoader.h"

#include <fstream>
#include <string>

#include <ctre.hpp>

double impl::asDouble(std::string_view sv) {
  return std::stod(std::string(sv)); // This is dreadful
}

size_t impl::asIndex(std::string_view sv, size_t max) {
  auto res = std::stol(std::string(sv));
  return res < 0 ? res + max : res;
}

std::unordered_map<std::string, Material>
impl::loadMaterials(std::istream &in) {
  using namespace std::literals;
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
