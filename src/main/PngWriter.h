#pragma once

#include <cstdint>
#include <cstdio>
#include <png.h>
#include <memory>

class PngWriter {
  std::unique_ptr<FILE, decltype(fclose) *> file_;
  png_structp pngStruct_{};
  png_infop info_{};

public:
  PngWriter(const char *filename, int width, int height);
  ~PngWriter();

  PngWriter(const PngWriter &) = delete;
  PngWriter &operator=(const PngWriter &) = delete;
  PngWriter(PngWriter &&) = delete;
  PngWriter &operator=(PngWriter &&) = delete;

  void addRow(const uint8_t *rowData);

  bool ok() const { return pngStruct_ && info_; }
};
