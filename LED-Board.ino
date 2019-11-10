#define LOAD 10
#define DATA 8
#define CLOCK 9

#define MAX_WIDTH 32
#define MAX_HEIGHT 40

typedef struct image_t {
  int width;
  int height;
  byte data[MAX_WIDTH * MAX_HEIGHT];
};

image_t img;

byte get_pixel(image_t* p, int x, int y) {
  if(check_bounds(p, x, y) == false) {
    return 0;
  }
  return p->data[y*p->width + x];
}

void set_pixel(image_t* p, int x, int y, byte value) {
  if(check_bounds(p, x, y) == false) {
    return;
  }
  p->data[y*p->width + x] = value;
}

bool check_bounds(image_t* p, int x, int y) {
  if(p == NULL) {
    return false;
  }

  if((x < 0) || (y < 0)) {
    return false;
  }

  if((x > p->width) || (y > p->height)) {
    return false;
  }

  return true;
}

void clear_pixels(image_t* p) {
  if(p == NULL) {
    return;
  }
  memset(p->data, 0, sizeof(p->data));
}

void setup() {
  Serial.begin(115200);

  img.width = 32;
  img.height = 40;

  pinMode(DATA,OUTPUT);
  pinMode(CLOCK,OUTPUT);
  pinMode(LOAD,OUTPUT);
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

  for(int n = 0; n < 32; n++) {
    int x_offset = order[n][0];
    int y_offset = order[n][1];

    byte pixel = get_pixel(p, x + x_offset, y + y_offset);
    digitalWrite(DATA, pixel);
    clock();
  } 

  // 33 bit - kein pixel - senden
  clock();
}

void send_image(image_t* p) {
  for(int y = 0; y < img.height; y+=8) {
    for(int x = 0; x < img.width; x+=4) {
      send_block(p, x, y);
    }
  }
  
  load();
}

void clock() {
  digitalWrite(CLOCK,HIGH);
  digitalWrite(CLOCK,LOW);
}

void load() {
  digitalWrite(LOAD,HIGH);
  digitalWrite(LOAD,LOW);
}

void loop() {
  /*
  static int offset = 0;
  
  digitalWrite(DATA, LOW);
  for(int i = 0; i < 1320; i++) {
    clock();
  }

  digitalWrite(DATA, HIGH);
  for(int i = 0; i < 33; i++) {
    clock();
  }

  digitalWrite(DATA, LOW);
  for(int i = 0; i < offset; i++) {
    clock();
  }
  offset++;
  */


  clear_pixels(&img);
  /*
  static int offset = 0;
  for(int n = 0; n < 40; n++) {
    set_pixel(&img, (n + offset) % 32, n, 1);
    set_pixel(&img, (n + 16 + offset) % 32, n, 1);
  }
  offset = (offset + 1) % 33;
  */
  
  memcpy(&(img.data), &invader, sizeof(invader));
  
  
  /*
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 8; y++) {
      set_pixel(&img, x, y, 1);
    }
  }
  */
  send_image(&img);

  //delay(100);
}
