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
  return p->data[y*p->width + x];
}

byte set_pixel(image_t* p, int x, int y, byte value) {
  p->data[y*p->width + x] = value;
}

void setup() {
  Serial.begin(115200);

  img.width = 32;
  img.height = 40;
  
  // put your setup code here, to run once:
  byte pixel = 1;
  for(int y = 0; y < img.height; y++) {
    pixel = y % 2;
    for(int x = 0; x < img.width; x++) {
      set_pixel(&img, x, y, pixel);
      pixel = !pixel;
    } 
  }

  pinMode(DATA,OUTPUT);
  pinMode(CLOCK,OUTPUT);
  pinMode(LOAD,OUTPUT);
}

void send_block(image_t* p, int x, int y) {
  int order[32][2] = {
     { 6, 2 }, // 32
     { 7, 2 }, // 31
     { 6, 3 }, // 30
     { 5, 2 }, // 29
     { 5, 3 }, // 28
     { 4, 2 }, // 27
     { 7, 3 }, // 26
     { 4, 3 }, // 25
     { 3, 3 }, // 24
     { 3, 2 }, // 23
     { 2, 3 }, // 22
     { 0, 2 }, // 21
     { 2, 2 }, // 20
     { 1, 3 }, // 19
     { 1, 2 }, // 18
     { 0, 3 }, // 17
     { 0, 0 }, // 16
     { 1, 1 }, // 15
     { 0, 1 }, // 14
     { 1, 0 }, // 13
     { 2, 1 }, // 12
     { 2, 0 }, // 11
     { 3, 0 }, // 10
     { 3, 1 }, // 9
     { 4, 0 }, // 8
     { 7, 1 }, // 7
     { 7, 0 }, // 6
     { 4, 1 }, // 5
     { 5, 0 }, // 4
     { 6, 1 }, // 3
     { 6, 0 }, // 2
     { 5, 1 } // 1
  };

  Serial.println("start block");
  for(int n = 0; n < 32; n++) {
    int x_offset = order[n][1];
    int y_offset = order[n][0];

    byte pixel = get_pixel(p, x + x_offset, y + y_offset);
    Serial.print("x=");
    Serial.print(x_offset);
    Serial.print(" y=");
    Serial.print(y_offset);
    Serial.print(" pixel=");
    Serial.println(pixel);
    
    
    digitalWrite(DATA, pixel);
    clock();
  } 

  // 33 bit - kein pixel - senden
  clock();
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
  digitalWrite(DATA, LOW);
  for(int i = 0; i < 1320; i++) {
    clock();
  }

  for(int y = 0; y < img.height; y+=8) {
    for(int x = 0; x < img.width; x+=4) {
      send_block(&img, x, y);
    }
  }
  
  load();
}
