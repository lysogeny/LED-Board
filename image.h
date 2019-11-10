#include <Arduino.h>

#define MAX_WIDTH 32
#define MAX_HEIGHT 40

typedef struct image_t {
  int width;
  int height;
  byte data[MAX_WIDTH * MAX_HEIGHT];
};

byte get_pixel(image_t* p, int x, int y);

void set_pixel(image_t* p, int x, int y, byte value);

void clear_pixels(image_t* p);
