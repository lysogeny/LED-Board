#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <Arduino.h>

#define MAX_WIDTH 64
#define MAX_HEIGHT 40

class Image
{
  public:
    byte get_pixel(int x, int y);
    void set_pixel(int x, int y, byte value);
    void clear_pixels();
    void set_size(int w, int h);
    uint16_t getWidth();
    uint16_t getHeight();

  private:
    bool check_bounds(int x, int y);

    int width;
    int height;
    byte data[MAX_WIDTH * MAX_HEIGHT];

};

#endif