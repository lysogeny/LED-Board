#include "image.hpp"


bool Image::check_bounds(int x, int y) {
  if ((x < 0) || (y < 0)) {
    return false;
  }

  if ((x >= width) || (y >= height)) {
    return false;
  }

  return true;
}

byte Image::get_pixel(int x, int y) {
  if (check_bounds(x, y) == false) {
    return 0;
  }
  return data[y * width + x];
}

void Image::set_pixel(int x, int y, byte value) {
  if (check_bounds(x, y) == false) {
    return;
  }
  data[y * width + x] = value;
}

void Image::clear_pixels() {
  memset(data, 0, sizeof(data));
}

void Image::set_size(int w, int h) {
  width = min(w, MAX_WIDTH);
  height = min(h, MAX_HEIGHT);
}

uint16_t Image::getWidth()
{
  return width;
}

uint16_t Image::getHeight()
{
  return height;
}