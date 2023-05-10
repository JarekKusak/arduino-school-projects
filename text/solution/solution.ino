#include "funshield.h"
#include "input.h"
unsigned long lastTime;
// map of letter glyphs
constexpr byte LETTER_GLYPH[] {
  0b10001000,   // A
  0b10000011,   // b
  0b11000110,   // C
  0b10100001,   // d
  0b10000110,   // E
  0b10001110,   // F
  0b10000010,   // G
  0b10001001,   // H
  0b11111001,   // I
  0b11100001,   // J
  0b10000101,   // K
  0b11000111,   // L
  0b11001000,   // M
  0b10101011,   // n
  0b10100011,   // o
  0b10001100,   // P
  0b10011000,   // q
  0b10101111,   // r
  0b10010010,   // S
  0b10000111,   // t
  0b11000001,   // U
  0b11100011,   // v
  0b10000001,   // W
  0b10110110,   // ksi
  0b10010001,   // Y
  0b10100100,   // Z
};
constexpr byte EMPTY_GLYPH = 0b11111111;

class Display {
  private:
    const int positionsCount = 4;
    const unsigned int scrollingInterval = 300;
    const int orderCount = sizeof(digit_muxpos) / sizeof(digit_muxpos[0]);
    char* message[4] = {" "," "," "," "};
    unsigned long lastScrollTime = 0;
    int displayIndex = 3;
    int messageIndex = 0;
    int timer = 0;
    int space[1] = {" "};

  public:
    void setupDisplay() {
      pinMode(latch_pin, OUTPUT);
      pinMode(clock_pin, OUTPUT);
      pinMode(data_pin, OUTPUT);
    }

    void displayChar(char ch, byte pos) {
      byte glyph = EMPTY_GLYPH;
      if (isAlpha(ch)) {
        glyph = LETTER_GLYPH[ ch - (isUpperCase(ch) ? 'A' : 'a') ];
      }

      digitalWrite(latch_pin, LOW);
      shiftOut(data_pin, clock_pin, MSBFIRST, glyph);
      shiftOut(data_pin, clock_pin, MSBFIRST, pos);
      digitalWrite(latch_pin, HIGH);
    }

    void displayString(const char* text, long deltaTime) {
      timer+=deltaTime;
      // Update message
      if (timer >= scrollingInterval) {
        timer=0;
        messageIndex++;
        if (text[messageIndex] == '\0') 
          messageIndex = 0;
        for (int i = 0; i < positionsCount; i++) 
          message[i] = text[messageIndex + i]; 
      }
      
      // Display message
      displayChar(message[displayIndex], digit_muxpos[displayIndex]);
      displayIndex--;
      if (displayIndex < 0) 
        displayIndex = 3;     
    }
};

SerialInputHandler input;

Display display;

void setup() {
  display.setupDisplay();
  input.initialize();
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - lastTime;
  display.displayString(input.getMessage(), deltaTime);
  input.updateInLoop();
  lastTime = currentTime;
}
