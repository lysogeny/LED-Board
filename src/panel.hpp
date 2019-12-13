#ifndef PANEL_HPP
#define PANEL_HPP

#include "image.hpp"

class Panel
{
    public:
    void init();
    void send_image(Image* img);

    private:
    void clock();
    void load();
    void send_block(Image* p, int x, int y);
};

#endif