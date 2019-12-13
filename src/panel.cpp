#include "panel.hpp"

#define LOAD 7
#define DATA 8
#define CLOCK 9

void Panel::init()
{
    pinMode(DATA, OUTPUT);
    pinMode(CLOCK, OUTPUT);
    pinMode(LOAD, OUTPUT);
}

void Panel::send_image(Image* img) {
  for (int y = 0; y < MAX_HEIGHT; y += 8) {
    for (int x = 0; x < MAX_WIDTH; x += 4) {
      send_block(img, x, y);
    }
  }

  load();
}


void Panel::clock() {
  PORTH |= (1 << PH6);
  PORTH &= ~(1 << PH6);
  //digitalWrite(CLOCK, HIGH);
  //digitalWrite(CLOCK, LOW);
}

void Panel::load() {
  PORTH |= (1 << PH4);
  PORTH &= ~(1 << PH4);
  //digitalWrite(LOAD, HIGH);
  //digitalWrite(LOAD, LOW);
}


void Panel::send_block(Image* p, int x, int y) {
  int order[32][2] = {
    { 1, 1 }, // 1
    { 1, 0 }, // 2
    { 0, 1 }, // 3
    { 1, 2 }, // 4
    { 0, 2 }, // 5
    { 1, 3 }, // 6
    { 0, 0 }, // 7
    { 0, 3 }, // 8
    { 0, 4 }, // 9
    { 1, 4 }, // 10
    { 0, 5 }, // 11
    { 1, 7 }, // 12
    { 1, 5 }, // 13
    { 0, 6 }, // 14
    { 1, 6 }, // 15
    { 0, 7 }, // 16
    { 3, 7 }, // 17
    { 2, 6 }, // 18
    { 2, 7 }, // 19
    { 3, 6 }, // 20
    { 2, 5 }, // 21
    { 3, 5 }, // 22
    { 3, 4 }, // 23
    { 2, 4 }, // 24
    { 3, 3 }, // 25
    { 2, 0 }, // 26
    { 3, 0 }, // 27
    { 2, 3 }, // 28
    { 3, 2 }, // 29
    { 2, 1 }, // 30
    { 3, 1 }, // 31
    { 2, 2 }, // 32
  };

  for (int n = 0; n < 32; n++) {
    int x_offset = order[n][0];
    int y_offset = order[n][1];

    byte pixel = p->get_pixel(x + x_offset, y + y_offset);
    
    if(pixel)
    {
      PORTH |= (1 << PH5);
    }
    else
    {
      PORTH &= ~(1 << PH5);
    }
    
    //digitalWrite(DATA, pixel);
    clock();
  }

  // 33 bit - kein pixel - senden
  clock();
}

