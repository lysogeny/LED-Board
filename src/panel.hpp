#ifndef PANEL_HPP
#define PANEL_HPP

#include "image.hpp"

#define PANEL_WIDTH 32
#define PANEL_HEIGHT 40

class Panel
{
    public:
    Panel(uint8_t pData, uint8_t pClock, uint8_t pLoad, int x, int y);
    void init();
    void send_image(Image* img);

    private:
    void clock();
    void load();
    void send_block(Image* p, int x, int y);

    uint8_t pinData;
    uint8_t pinClock;
    uint8_t pinLoad;

    // position of panel in image
    int posX;
    int posY;
};

#endif