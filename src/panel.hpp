#ifndef PANEL_HPP
#define PANEL_HPP

#include "image.hpp"

class Panel
{
    public:
    Panel(uint8_t pData, uint8_t pClock, uint8_t pLoad);
    void init();
    void send_image(Image* img);

    private:
    void clock();
    void load();
    void send_block(Image* p, int x, int y);
    uint8_t pinData;
    uint8_t pinClock;
    uint8_t pinLoad;
};

#endif