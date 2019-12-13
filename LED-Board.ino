#include <Arduino.h>
#include <Ethernet.h>

#include "src/WebSocketsServer.h"
#include "src/image.hpp"
#include "src/panel.hpp"
#include "src/ProtocolDL.hpp"

#define USE_SERIAL Serial

byte mac[] = { 0xBE, 0xB7, 0x5C, 0x30, 0xC3, 0x04 };
IPAddress ip(10, 23, 42, 24);
IPAddress router(10, 23, 42, 1);
IPAddress subnet(255, 255, 254, 0);

WebSocketsServer webSocket = WebSocketsServer(81);

Image image;
Panel panel1(22, 24, 23, 0 * PANEL_WIDTH, 0 * PANEL_HEIGHT);       //data, clock, load
Panel panel2(28, 29, 31, 1 * PANEL_WIDTH, 0 * PANEL_HEIGHT);
ProtocolDL protocol = ProtocolDL(image);

unsigned long last_activity = 0;

bool someOneIsConnected = false;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    static bool in_header = true;

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.print("[");
            USE_SERIAL.print(num);
            USE_SERIAL.println("] Disconnected!");
            someOneIsConnected = false;
            break;
        case WStype_CONNECTED:
            {
                //IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.print("[");
                USE_SERIAL.print(num);
                USE_SERIAL.print("] Connected ");
                USE_SERIAL.print(" url: ");
                //USE_SERIAL.println(payload);
				
				// send message to client
				webSocket.sendTXT(num, "Connected");
                someOneIsConnected = true;
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.print("[");
            USE_SERIAL.print(num);
            USE_SERIAL.print("] get Text: ");
            //USE_SERIAL.println(payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.print("[");
            USE_SERIAL.print(num);
            USE_SERIAL.print("] get binary length: ");
            USE_SERIAL.println(length);
            
            for(uint16_t i = 0; i < length; i++)
            {
              protocol.newByte(payload[i]);
            }

            if(protocol.isComplete())
            {
              Serial.println("complete");
              panel1.send_image(&image);
              panel2.send_image(&image);
            }

            break;
    }

}

void setup() {
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    //USE_SERIAL.setDebugOutput(true);

    Ethernet.init(10);
    Ethernet.begin(mac, ip, router, router, subnet);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.print("[SETUP] BOOT WAIT ");
        USE_SERIAL.print(t);
        USE_SERIAL.println("...");
        USE_SERIAL.flush();
        delay(1000);
    }

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    panel1.init();
    panel2.init();

    Serial.println("setup done");
}



// 0x00 0x00 0x00 0x00 0x00 0x00 0x00...
// Width     Height    Delay     Pixel

void default_image(Image* p) {
  static int offset = 0;

  // reset image to maximum size
  p->set_size(32767, 32767);

  // toggle all pixels in tilted bars
  int dim = max(p->getWidth(), p->getHeight());
  for (int n = 0; n < dim; n++) {
    int x = (n + offset) % p->getWidth();
    int y = n % p->getHeight();

    byte pixel = p->get_pixel(x, y);
    p->set_pixel(x, y, !pixel);
  }
  offset++;
}

void loop() {
    webSocket.loop();

    if (someOneIsConnected == false) {
        default_image(&image);
        panel1.send_image(&image);
        panel2.send_image(&image);
    }
}
