#include <Arduino.h>

#define MAX_WIDTH 32
#define MAX_HEIGHT 40

struct _image_t {
  int width;
  int height;
  byte data[MAX_WIDTH * MAX_HEIGHT];
};

typedef struct _image_t image_t;

byte get_pixel(image_t* p, int x, int y);

void set_pixel(image_t* p, int x, int y, byte value);

void clear_pixels(image_t* p);

void set_size(image_t* p, int width, int height);
