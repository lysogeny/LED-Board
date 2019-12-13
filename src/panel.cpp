#include "panel.hpp"

Panel::Panel(uint8_t pData, uint8_t pClock, uint8_t pLoad, int x, int y)
{
  pinData = pData;
  pinLoad = pLoad;
  pinClock = pClock;

  posX = x;
  posY = y;
}

void Panel::init()
{
    pinMode(pinData, OUTPUT);
    pinMode(pinClock, OUTPUT);
    pinMode(pinLoad, OUTPUT);
}

void Panel::send_image(Image* img) {
  int endY = posY + PANEL_HEIGHT;
  int endX = posX + PANEL_WIDTH;

  for (int y = posY; y < endY; y += 8) {
    for (int x = posX; x < endX; x += 4) {
      send_block(img, x, y);
    }
  }

  load();
}


void Panel::clock() {
  //PORTA |= (1 << PA2);
  //PORTA &= ~(1 << PA2);
  digitalWrite(pinClock, HIGH);
  digitalWrite(pinClock, LOW);
}

void Panel::load() {
  //PORTB |= (1 << PB0);
  //PORTB &= ~(1 << PB0);
  digitalWrite(pinLoad, HIGH);
  digitalWrite(pinLoad, LOW);
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
    
    /*if(pixel)
    {
      PORTB |= (1 << PB2);
    }
    else
    {
      PORTB &= ~(1 << PB2);
    }*/
    
    digitalWrite(pinData, pixel);
    clock();
  }

  // 33 bit - kein pixel - senden
  clock();
}

