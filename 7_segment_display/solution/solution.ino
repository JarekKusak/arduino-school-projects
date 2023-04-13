#include "funshield.h"

constexpr int buttonPins[] { button1_pin, button2_pin, button3_pin };
constexpr int buttonPinsCount = sizeof(buttonPins) / sizeof(buttonPins[0]);
constexpr int displayDigits = 4;

void printNumber(int n){
  shiftOut(data_pin, clock_pin, MSBFIRST, 0x92);   // Display digit 7 (11111000) ...
  shiftOut(data_pin, clock_pin, MSBFIRST, 5);     // ... on positions 1 and 3 (0101)
  digitalWrite(latch_pin, LOW);                   // Trigger the latch
  digitalWrite(latch_pin, HIGH);
  delay(300);
}


void setup() {
  for (int i = 0; i < buttonPinsCount; ++i) {
    pinMode(buttonPins[i], INPUT);
  }

  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);


}
int counter = 0;
void loop() {
  
  printNumber(counter++);
  counter=counter%10;
  
}
