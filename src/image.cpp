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

void Image::set_pixel_offset(int offset, byte value) {
  int x = offset % IMAGE_WIDTH;
  int y = floor(offset / IMAGE_WIDTH);
  if (check_bounds(x, y) == false) {
    Serial.print(F("set_pixel outOfBound"));
    Serial.print(x);
    Serial.print("x");
    Serial.print(y);
    Serial.print("\r\n");
    return;
  }

  if (offset >= IMAGE_BUFFER) {
    Serial.print(F("buffer out of index ["));
    Serial.print(offset);
    Serial.print(F("] : "));
    Serial.print(x);
    Serial.print("x");
    Serial.print(y);
    Serial.print("\r\n");
    return;
  }

  data[offset] = value;
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
