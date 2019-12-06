#include "image.h"

bool check_bounds(image_t* p, int x, int y) {
  if (p == NULL) {
    return false;
  }

  if ((x < 0) || (y < 0)) {
    return false;
  }

  if ((x >= p->width) || (y >= p->height)) {
    return false;
  }

  return true;
}

byte get_pixel(image_t* p, int x, int y) {
  if (check_bounds(p, x, y) == false) {
    return 0;
  }
  return p->data[y * p->width + x];
}

void set_pixel(image_t* p, int x, int y, byte value) {
  if (check_bounds(p, x, y) == false) {
    return;
  }
  p->data[y * p->width + x] = value;
}

void clear_pixels(image_t* p) {
  if (p == NULL) {
    return;
  }
  memset(p->data, 0, sizeof(p->data));
}

void set_size(image_t* p, int width, int height) {
  p->width = min(width, MAX_WIDTH);
  p->height = min(height, MAX_HEIGHT);
}
