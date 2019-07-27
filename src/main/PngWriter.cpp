#include "PngWriter.h"

PngWriter::PngWriter(const char *filename, int width, int height)
    : f(fopen(filename, "wb"), fclose) {
  png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (!png_ptr)
    return;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    return;

  png_init_io(png_ptr, f.get());

  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
               PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);
}

PngWriter::~PngWriter() {
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);
}

void PngWriter::addRow(const uint8_t *rowData) {
  const png_byte *rowPointers[] = {reinterpret_cast<const png_byte *>(rowData)};
  png_write_rows(png_ptr, const_cast<png_byte **>(rowPointers), 1);
}
