#include "funshield.h"
#include "input.h"
unsigned long lastTime;
// map of letter glyphs
constexpr byte LETTER_GLYPH[]{
  0b10001000,  // A
  0b10000011,  // b
  0b11000110,  // C
  0b10100001,  // d
  0b10000110,  // E
  0b10001110,  // F
  0b10000010,  // G
  0b10001001,  // H
  0b11111001,  // I
  0b11100001,  // J
  0b10000101,  // K
  0b11000111,  // L
  0b11001000,  // M
  0b10101011,  // n
  0b10100011,  // o
  0b10001100,  // P
  0b10011000,  // q
  0b10101111,  // r
  0b10010010,  // S
  0b10000111,  // t
  0b11000001,  // U
  0b11100011,  // v
  0b10000001,  // W
  0b10110110,  // ksi
  0b10010001,  // Y
  0b10100100,  // Z
};
constexpr byte EMPTY_GLYPH = 0b11111111;
SerialInputHandler input;

class Display {

private:
  const int orderCount = sizeof(digit_muxpos) / sizeof(digit_muxpos[0]);
  int timer = 0;
  int scrollingInterval = 300;
  int messageLength = 0;
  int messageIndex = -orderCount;
  int position = 3;
  const char* message;
  bool messageReceived = false;

public:
 
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

  int length(const char* string) {
    int i = 0;
    while (string[i])
      i++;
    return i;
  }
  
  void displayMessage(long deltaTime) {
    timer+=deltaTime;
    if (timer >= scrollingInterval) {
      timer -= scrollingInterval;
      messageIndex++;
      if (messageIndex >= messageLength) {
        messageIndex = -orderCount;
        messageReceived = false;
      }
    }
    displayChar((messageIndex + position < 0) || (messageIndex + position > messageLength) ? EMPTY_GLYPH : message[messageIndex + position], digit_muxpos[position]);
    position--;
    if (position < 0) 
      position = orderCount-1;
  }

  bool checkIfMessageReceived() {
    if (!messageReceived) { 
      message = input.getMessage();
      messageLength = length(message);
      messageReceived = true;
    }
    return messageReceived;
  }
}display;

void setup() {
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);

  input.initialize();
};

void loop() {
  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - lastTime;
  input.updateInLoop();
  if (display.checkIfMessageReceived())
    display.displayMessage(deltaTime);
  lastTime = currentTime;
}
