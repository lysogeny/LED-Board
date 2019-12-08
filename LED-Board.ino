#include <Arduino.h>
#include <Ethernet.h>

#include "src/WebSocketsServer.h"
#include "src/image.hpp"
#include "src/ProtocolDL.hpp"

#define USE_SERIAL Serial


#define LOAD 7
#define DATA 8
#define CLOCK 9


byte mac[] = { 0xBE, 0xB7, 0x5C, 0x30, 0xC3, 0x04 };
IPAddress ip(10, 23, 42, 24);
IPAddress router(10, 23, 42, 1);
IPAddress subnet(255, 255, 254, 0);

WebSocketsServer webSocket = WebSocketsServer(81);

Image image;
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
              send_image(&image);
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

    pinMode(DATA, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(LOAD, OUTPUT);

  Serial.println("setup done");
}


void send_block(Image* p, int x, int y) {
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
    digitalWrite(DATA, pixel);
    clock();
  }

  // 33 bit - kein pixel - senden
  clock();
}

void send_image(Image* img) {
  for (int y = 0; y < MAX_HEIGHT; y += 8) {
    for (int x = 0; x < MAX_WIDTH; x += 4) {
      send_block(img, x, y);
    }
  }

  load();
}

void clock() {
  digitalWrite(CLOCK, HIGH);
  digitalWrite(CLOCK, LOW);
}

void load() {
  digitalWrite(LOAD, HIGH);
  digitalWrite(LOAD, LOW);
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
        send_image(&image);
    }
}
