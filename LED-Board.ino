#include <Ethernet.h>
#include "image.h"

#define LOAD 7
#define DATA 8
#define CLOCK 9

typedef struct source_t {
  unsigned int width;
  unsigned int height;
  unsigned int delay;

  int x;
  int y;
};

byte mac[] = { 0xBE, 0xB7, 0x5C, 0x30, 0xC3, 0x04 };
IPAddress ip(10, 23, 42, 24);
IPAddress router(10, 23, 42, 1);
IPAddress subnet(255, 255, 254, 0);

EthernetServer server(9000);


image_t image;

source_t source;

void setup() {
  // setup network
  Ethernet.init(10);
  Ethernet.begin(mac, ip, router, router, subnet);
  server.begin();

  Serial.begin(115200);

  image.width = 32;
  image.height = 40;

  pinMode(DATA, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(LOAD, OUTPUT);
}

void send_block(image_t* p, int x, int y) {
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

    byte pixel = get_pixel(p, x + x_offset, y + y_offset);
    digitalWrite(DATA, pixel);
    clock();
  }

  // 33 bit - kein pixel - senden
  clock();
}

void send_image(image_t* img) {
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

void default_image(image_t* p) {
  static int offset = 0;

  int dim = max(p->width, p->height);
  for (int n = 0; n < dim; n++) {
    int x = (n + offset) % p->width;
    int y = n % p->height;

    byte pixel = get_pixel(p, x, y);
    set_pixel(p, x, y, !pixel);
  }
  offset++;
}

bool read_header(EthernetClient cli, source_t* src) {
  // number of bytes already read from header
  static int offset = 0;
  // flag set, if header is complete
  bool complete = false;

  while (offset < 6) {
    int value = cli.read();
    if (value == -1) {
      break;
    }

    switch (offset) {
      case 0:
        src->width = (value << 8);
        break;
      case 1:
        src->width |= value;
        break;
      case 2:
        src->height = (value << 8);
        break;
      case 3:
        src->height |= value;
        break;
      case 4:
        src->delay = (value << 8);
        break;
      case 5:
        src->delay |= value;
        break;
    }

    offset++;
  }

  if (offset > 5) {
    offset = 0;
    complete = true;
  }

  return complete;
}

bool read_pixels(EthernetClient cli, source_t* src, image_t* img) {
  // position of current pixel
  static int x = 0;
  static int y = 0;

  // copy dimension from header
  img->width = src->width;
  img->height = src->height;

  while (true) {
    int value = cli.read();
    if (value == -1) {
      return false;
    }

    set_pixel(img, x, y, value);

    x++;
    if (x >= img->width) {
      x = 0;
      y++;
      if (y >= img->height) {
        y = 0;
        return true;
      }
    }
  }
}

void loop() {
  static bool seen_client = false;
  static bool in_header = true;

  // if an incoming client connects, there will be bytes available to read:
  EthernetClient client = server.available();
  if (client) {
    if (in_header == true) {
      if (read_header(client, &source) == true) {
        Serial.print("width=");
        Serial.print(source.width);
        Serial.print(" height=");
        Serial.print(source.height);
        Serial.print(" delay=");
        Serial.println(source.delay);
        in_header = false;
      }
    } else {
      if (read_pixels(client, &source, &image) == true) {
        Serial.println("pixels complete");
        in_header = true;

        send_image(&image);
      }
    }

    seen_client = true;
  } else {
    //if (seen_client == false) {
      default_image(&image);
      send_image(&image);
    //}
  }
}
