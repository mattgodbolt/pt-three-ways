#pragma once

#include <cstdint>
#include <cstdio>
#include <libpng/png.h>
#include <memory>

class PngWriter {
  std::unique_ptr<FILE, decltype(fclose) *> f;
  png_structp png_ptr{};
  png_infop info_ptr{};

public:
  PngWriter(const char *filename, int width, int height);
  ~PngWriter();
  PngWriter(const PngWriter &) = delete;
  PngWriter &operator=(const PngWriter &) = delete;
  PngWriter(PngWriter &&) = delete;
  PngWriter &operator=(PngWriter &&) = delete;

  void addRow(const uint8_t *rowData);

  bool ok() const { return png_ptr && info_ptr; }
};
