#include "funshield.h"
unsigned long lastTime;
unsigned long number = 0;
constexpr long SHIFT = 100;
constexpr long MAX_NUMBER = 10000*SHIFT; // 10000*100 (SHIFT) 

class Button {
  private:
    int pin;
    bool pressed;
    bool wasPressed;
  public:
  Button(int button_pin) {
    pin = button_pin;
    pressed = OFF;
    wasPressed = OFF;
  }
  void setupPin() {
    pinMode(pin, INPUT);
  }
  
  bool wasButtonPressed() {
    bool buttonState = digitalRead(pin);
    pressed = buttonState;
    bool isBeingPressed = false;
    if (pressed == ON) {
      if (wasPressed == OFF) // if button wasn't holded in previous iteration
        isBeingPressed = true;
      wasPressed = ON; // sets last button's state as held
    }
    else
       wasPressed = OFF;
    return isBeingPressed;
  }
};

class Display {
  private:
    const int tenths = 8; 
    const int ones = 4;
    const int tens = 2;
    const int hundreds = 1;
    const int ONE_SECOND = 1000;
    long timer = 0;
    int orderCount = sizeof(digit_muxpos) / sizeof(digit_muxpos[0]);
    int i = 0;
    bool start = false;
  public:
  
  void displayOutput(int order, int digit) {
    if (digit == 0 && order == tens && timer < 10000)
      return;
    if (digit == 0 && order == hundreds && timer < 100000)
      return;
    
    shiftOut(data_pin, clock_pin, MSBFIRST, (order == ones) ? digits[digit]-128 : digits[digit]); // if order is at ones, show decimal point
    shiftOut(data_pin, clock_pin, MSBFIRST, order); // ... on positions 1 and 3 (0101)
    digitalWrite(latch_pin, LOW); // Trigger the latch
    digitalWrite(latch_pin, HIGH);
  }

  int getDigitAtPosition(long number, int position) {
    long multiplier = pow(10, position);
    return (number / multiplier) % 10;
  }
  
  bool show = true;
  long temp;
  void showNumber(bool firstButtonWasPressed, bool secondButtonWasPressed, bool thirdButtonWasPressed, int deltaTime) {
    if (firstButtonWasPressed && !start)
      start = true;
    else if (firstButtonWasPressed && start && show)
      start = false;

    if (start && secondButtonWasPressed && show)
       show = false;
    else if (start && secondButtonWasPressed && !show)
       show = true;
    
    if (!start && thirdButtonWasPressed)
      timer = 0;
        
    if (start)
      startCounting(deltaTime); 
    
    if (show) {
      temp = timer;
      displayOutput(digit_muxpos[orderCount-i-1], getDigitAtPosition(timer/SHIFT, i));
    }
    else 
      displayOutput(digit_muxpos[orderCount-i-1], getDigitAtPosition(temp/SHIFT, i));
         
    i++;
    i%=orderCount;
  }

  void startCounting(long deltaTime) {
    timer+=deltaTime;
    timer = timer%MAX_NUMBER;
  }
  
} display;

Button button1(button1_pin); // start/stop stopwatches
Button button2(button2_pin); // freeze while running
Button button3(button3_pin); // restart while not running
Button buttons[3] = {button1, button2, button3};
constexpr int buttonsCount= sizeof(buttons) / sizeof(buttons[0]); // number of buttons 

void setup() {
  for (int i = 0; i < buttonsCount; i++)
    buttons[i].setupPin();
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  lastTime = millis(); // time at start 
  Serial.begin(9600);
  Serial.println(MAX_NUMBER);
}

void loop() {
  unsigned long currentTime = millis(); // Time since start
  unsigned long deltaTime = 0; // Time since last loop
  deltaTime = currentTime - lastTime;
  display.showNumber(buttons[0].wasButtonPressed(), buttons[1].wasButtonPressed(), buttons[2].wasButtonPressed(), deltaTime);
  lastTime = currentTime;
}
