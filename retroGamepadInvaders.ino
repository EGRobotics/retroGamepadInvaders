#include <U8glib.h>
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

void softwareReset()
{
  asm volatile ("  jmp 0");  
} 

// UFO worth 50, 100, 150
// Aliens fire objects at player
// Bunkers may not be necessary?
// Use power-up with B button?
// control game speed with potentiometer?
// http://vignette1.wikia.nocookie.net/villains/images/9/9b/Spaceinvaders.png/revision/latest?cb=20130815215326

uint8_t invaderBig1[] U8G_PROGMEM = {
  B00000111, B10000000,
  B00111111, B11110000,
  B01110011, B00111000,
  B01111111, B11111000,
  B00011100, B11100000,
  B00110011, B00110000,
  B00011000, B01100000
};

uint8_t invaderBig2[] U8G_PROGMEM = {
  B00000111, B10000000,
  B00111111, B11110000,
  B01110011, B00111000,
  B01111111, B11111000,
  B00001100, B11000000,
  B00011011, B01100000,
  B01100000, B00011000
};
//
uint8_t invaderMed1[] U8G_PROGMEM = {
  B00001000, B00100000,
  B00000100, B01000000,
  B00001111, B11100000,
  B00011011, B10110000,
  B00111111, B11111000,
  B00101111, B11101000,
  B00101000, B00101000,
  B00000110, B11000000
};

uint8_t invaderMed2[] U8G_PROGMEM = {
  B00001000, B00100000,
  B00100100, B01001000,
  B00101111, B11101000,
  B00111011, B10111000,
  B00111111, B11111000,
  B00011111, B11110000,
  B00001000, B00100000,
  B00010000, B00010000
};

uint8_t invaderSml1[] U8G_PROGMEM = {
  B00000011, B00000000,
  B00000111, B10000000,
  B00001111, B11000000,
  B00011011, B01100000,
  B00011111, B11100000,
  B00000100, B10000000,
  B00001011, B01000000,
  B00010100, B10100000
};

uint8_t invaderSml2[] U8G_PROGMEM = {
  B00000011, B00000000,
  B00000111, B10000000,
  B00001111, B11000000,
  B00011011, B01100000,
  B00011111, B11100000,
  B00001011, B01000000,
  B00010000, B00100000,
  B00001000, B01000000,
};

uint8_t ufo[] U8G_PROGMEM = {
  B00000111, B11100000,
  B00011111, B11111000,
  B00111111, B11111100,
  B01101101, B10110110,
  B11111111, B11111111,
  B00111001, B10011100,
  B00010000, B00001000,
};

uint8_t gun1[] U8G_PROGMEM = {
  B00000001, B00000000,
  B00001111, B11100000,
  B00001111, B11100000,
  B00011111, B11110000
};

int ceiling(float num) {
  if (num - (int) num) {
    return (int) (num + 1);
  }
  return (int) num;
}

bool gameIsOver = false;
void gameOver() {
  gameIsOver = true;
};

int DPAD_PUSH = 9, DPAD_DOWN = 10, DPAD_RIGHT = 11, DPAD_UP = 12, DPAD_LEFT = 13, A = 7, B = 8, POT_HIGH = 2, POT_LOW = 3, POT = 3, potRead = 0, totalAliens = 18;
bool pushIsPressed = false, downIsPressed = false, rightIsPressed = false, upIsPressed = false, leftIsPressed = false, aIsPressed = false, bIsPressed = false;

struct Bullet {
  float velocity, y;
  int x, height, exist = 0;
  Bullet(int xpos, int ypos, int newheight, float newvelocity) {
    x = xpos;
    y = ypos;
    height = newheight;
    velocity = newvelocity;
  }
  void draw() {
    if (exist > 0 && y > 0) {
      y -= velocity;
      u8g.drawVLine(x, round(y), height);
    } else {
      exist = 0;
    }
  }
  bool isIn(int left, int top, int right, int bottom) {
    if (exist > 0 && x > left && x < right && y > top && y < bottom) {
      return true;
    }
    return false;
  }
};

Bullet *bul;

struct Cannon {
  const u8g_pgm_uint8_t *frame;
  float velocity = 2, x;
  int y = 0, width = 16, height = 8, bytewidth;
  Cannon(int theWidth, int theHeight, uint8_t frame1[]) {
    width = theWidth;
    bytewidth = ceiling(width / 8.0);
    height = theHeight;
    frame = frame1;
    y = 64 - height;
    x = (128 / 2) - (width / 2);
  }
  void moveLeft() {
    if (x > -1 * (width / 2)) {
      x -= velocity;
    }
  }
  void moveRight() {
    if (x < 126 - width / 2) {
      x += velocity;
    }
  }
  void draw() {
    u8g.drawBitmapP(round(x), y, bytewidth, height, frame);
  }
  void fire() {
    if (bul->exist < 1) {
      bul->exist = 1;
      bul->y = 64 - bul->height;
      bul->x = x + width / 2;
    }
  }
};

Cannon *gun;

int score;

struct Line {
  const u8g_pgm_uint8_t *frame;
  const u8g_pgm_uint8_t *frame1;
  const u8g_pgm_uint8_t *frame2;
  bool living[6] = {true, true, true, true, true, true};
  int x = 0, y = 0, width = 16, height = 8, dir = 4, frameNum = 1, bytewidth, value;
  Line(int setvalue, int xpos, int ypos, int theWidth, int theHeight, uint8_t frames1[], uint8_t frames2[]) {
    value = setvalue;
    width = theWidth;
    bytewidth = ceiling(width / 8.0);
    height = theHeight;
    frame1 = frames1;
    frame2 = frames2;
    x = xpos;
    y = ypos;
  }
  void inc() {
    frameNum = (frameNum + 1) % 2;
    if (x < 0 && dir < 0) {
      y += 5;
    }
    if (y + height > 64) {
      gameOver();
    }
    if (x + (width * 6) > 128 || x < 0) {
      dir *= -1;
    }
    x += dir;
  }
  void draw() {
    for (int i = 0; i < 6; i++) {
      if (living[i]) {
        int xpos = x + width * i;
        if (bul->isIn(xpos, y, xpos + width, y + height)) {
          living[i] = false;
          totalAliens--;
          if(totalAliens == 0){
            reset();
            // next wave, make game harder
          }
          score += value;
          bul->exist = 0;
        } else {
          if (frameNum < 1) {
            frame = frame1;
          } else {
            frame = frame2;
          }
          u8g.drawBitmapP(xpos, y, bytewidth, height, frame);
        }
      }
    }
  }
};

Line *big, *med, *sml;

void reset(){
  sml = new Line(40, 0, 0, 14, 8, invaderSml1, invaderSml2);
  med = new Line(20, 0, 10, 14, 8, invaderMed1, invaderMed2);
  big = new Line(10, 0, 20, 14, 7, invaderBig1, invaderBig2);
  totalAliens = 18;
};

void setup()
{
  pinMode(POT_HIGH, OUTPUT);
  pinMode(POT_LOW, OUTPUT);
  pinMode(DPAD_PUSH, INPUT_PULLUP);
  pinMode(DPAD_DOWN, INPUT_PULLUP);
  pinMode(DPAD_RIGHT, INPUT_PULLUP);
  pinMode(DPAD_UP, INPUT_PULLUP);
  pinMode(DPAD_LEFT, INPUT_PULLUP);
  pinMode(A, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);
  Serial.begin(9600);
  // randomSeed is neccessary to get varying random numbers each time program starts
  int rand = analogRead(0);
  randomSeed(rand);
  reset();
  gun = new Cannon(14, 4, gun1);
  bul = new Bullet(-1, -4, 4, 0.5);
}

void hook(int buttonPin, bool& buttonState, String buttonName) {
  if (digitalRead(buttonPin) == LOW && !buttonState) {
    buttonState = true;
    Serial.println(buttonName + " pressed.");
  }
  if (digitalRead(buttonPin) == HIGH && buttonState) {
    buttonState = false;
    Serial.println(buttonName + " released.");
  }
}

void buttonHooks() {
  hook(DPAD_PUSH, pushIsPressed, "DPAD");
  hook(DPAD_DOWN, downIsPressed, "DPAD DOWN");
  hook(DPAD_RIGHT, rightIsPressed, "DPAD RIGHT");
  hook(DPAD_UP, upIsPressed, "DPAD UP");
  hook(DPAD_LEFT, leftIsPressed, "DPAD LEFT");
  hook(A, aIsPressed, "A");
  hook(B, bIsPressed, "B");
}

void potHook() {
  int reading = analogRead(POT);
  if (reading - potRead > 5 || reading - potRead < -5) {
    potRead = reading;
    Serial.println("Pot set to " + String(reading) + ".");
  }
  digitalWrite(POT_HIGH, HIGH);
  digitalWrite(POT_LOW, LOW);
}

unsigned long lastFrame = 0;

bool nextFrame() {
  if (millis() > lastFrame + 1000 / 4) {
    lastFrame = millis();
    return true;
  }
  return false;
}

void draw() {
  if (!gameIsOver) {
    if (nextFrame()) {
      big->inc();
      med->inc();
      sml->inc();
    }
    big->draw();
    med->draw();
    sml->draw();
    gun->draw();
    bul->draw();
  } else {
    //game ending screen
  }
}

void loop()
{
  buttonHooks();
  potHook();
  if (aIsPressed) {
    gun->fire();
    if(gameIsOver){
      softwareReset();
    }
  }
  if (leftIsPressed) {
    gun->moveLeft();
  }
  if (rightIsPressed) {
    gun->moveRight();
  }
  u8g.firstPage();
  do {
    draw();
  } while (u8g.nextPage());
  //  delay(1000 / 5);
}
