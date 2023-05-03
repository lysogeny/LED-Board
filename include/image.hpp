#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <Arduino.h>

#define MAXIMUM_PANELSIZE 5
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 40

#define IMAGE_WIDTH (PANEL_WIDTH * MAXIMUM_PANELSIZE)
#define IMAGE_HEIGHT PANEL_HEIGHT

#define IMAGE_BUFFER (IMAGE_WIDTH * IMAGE_HEIGHT)

class Image
{
  public:
    uint8_t get_pixel(int x, int y);
    void set_pixel(int x, int y, uint8_t value);
    void set_pixel_offset(int offset, uint8_t value);
    void clear_pixels();

  private:
    bool check_bounds(int x, int y);

    uint8_t data[IMAGE_BUFFER];

};

#endif
