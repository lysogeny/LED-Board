#ifndef PROTOCOL_DL_HPP
#define PROTOCOL_DL_HPP

#include "image.hpp"

class ProtocolDL
{
    public:
    ProtocolDL(Image& img);
    void newByte(uint8_t data);
    bool isComplete();

    private:


    struct source_t {
        // width and height of current frame
        unsigned int width;
        unsigned int height;

        // delay until next frame is shown
        unsigned int delay;

        // position of current pixel
        int x;
        int y;
    };

    source_t source;
    uint16_t cnt = 0;
    bool complete = true;
    Image* image;
};

#endif