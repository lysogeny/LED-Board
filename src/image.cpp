#include "image.hpp"


bool Image::check_bounds(int x, int y) {
  if ((x < 0) || (y < 0)) {
    return false;
  }

  if ((x >= IMAGE_WIDTH) || (y >= IMAGE_HEIGHT)) {
    return false;
  }

  return true;
}

byte Image::get_pixel(int x, int y) {
  if (check_bounds(x, y) == false) {
    Serial.print(F("get_pixel outOfBound\n"));
    return 1;
  }
  return data[y * IMAGE_WIDTH + x];
}

void Image::set_pixel(int x, int y, byte value) {
  if (check_bounds(x, y) == false) {
    Serial.print(F("set_pixel outOfBound\n"));
    return;
  }
  data[y * IMAGE_WIDTH + x] = value;
}

void Image::clear_pixels() {
  memset(data, 0, sizeof(data));
}
