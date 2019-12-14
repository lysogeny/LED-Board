#include "ProtocolDL.hpp"

ProtocolDL::ProtocolDL(Image& img):
    image(&img)
{

}

void ProtocolDL::newByte(uint8_t data)
{
    switch(cnt)
    {
        case 0:
            complete = false;
            source.width = (data << 8);
            cnt = 1;
            break;
        
        case 1:
            source.width |= data;
            cnt = 2;
            break;
        
        case 2:
            source.height = (data << 8);
            cnt = 3;
            break;
        
        case 3:
            source.height |= data;
            cnt = 4;
            break;

        case 4:
            source.delay = (data << 8);
            cnt = 5;
            break;
        
        case 5:
            source.delay |= data;
            image->set_size(source.width, source.height);
            source.x = 0;
            source.y = 0;
            cnt = 6;
            break;

        default:
            for (int shift = 7; shift >= 0; shift--) {
                byte pixel = (data >> shift) & 1;
                image->set_pixel(source.x, source.y, pixel);

                if(source.x == (source.width - 1) && source.y == (source.height - 1))
                {
                    //this was the last pixel
                    complete = true;
                    cnt = 0;
                }
                else
                {
                    source.x++;
                    if (source.x >= source.width) {
                        source.x = 0;
                        source.y++;
                        if (source.y >= source.height) {
                            source.y = 0;
                        }
                    }
                }
            }

            break;
    }

    
}

bool ProtocolDL::isComplete()
{
    return complete;
}