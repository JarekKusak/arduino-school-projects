#include "funshield.h"
unsigned long lastTime;
constexpr long SHIFT = 100;
constexpr long MAX_NUMBER = 10000*SHIFT; // 10000*100 (SHIFT) 

class Stopwatches {
  private:
    enum State { isRunning, stopped, lapped };
    State state = stopped;
    bool start = false;
    bool show = true;
    long timer = 0; // in miliseconds
    long lastSavedTime = 0;
  public:
  
  void startOrStopState() {
   if (state == stopped)
    state = isRunning;
   else if (state == isRunning)
    state = stopped;
  }
  
  void lappedState() {
    if (state == isRunning)
      state = lapped;
    else if (state == lapped)
      state = isRunning;
  }
  
  void resetTime() {
    if (state == stopped)
      timer = 0;
  }
  
  long returnTime(long deltaTime) {
    if (state == isRunning) {
      lastSavedTime = timer;
      startCounting(deltaTime);
    }
    else if (state == lapped)
      startCounting(deltaTime);
    return (state == isRunning || state == stopped) ? timer : lastSavedTime;
  }

  void startCounting(long deltaTime) {
    timer+=deltaTime;
    timer = timer%MAX_NUMBER;
  }
} stopwatches;

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
    const int decimalPoint = 128;
    int orderCount = sizeof(digit_muxpos) / sizeof(digit_muxpos[0]);
    int i = 0;
  public:
  void displayOutput(int order, int digit, long time) {
    for (int i = 0; i < orderCount-2; i++) { // if the order is higher than ones and leading zero is about to display, then don't display anything
      if (digit == 0 && order == digit_muxpos[i] && time < pow(10, orderCount+1-i)) 
        return;
    }
    shiftOut(data_pin, clock_pin, MSBFIRST, (order == digit_muxpos[2]) ? digits[digit]-decimalPoint : digits[digit]); // if order is at ones, show decimal point
    shiftOut(data_pin, clock_pin, MSBFIRST, order); // ... on positions 1 and 3 (0101)
    digitalWrite(latch_pin, LOW); // Trigger the latch
    digitalWrite(latch_pin, HIGH);
  }

  int getDigitAtPosition(long number, int position) {
    long multiplier = pow(10, position);
    return (number / multiplier) % 10;
  }
  
  void showNumber(long time) {
    displayOutput(digit_muxpos[orderCount-i-1], getDigitAtPosition(time/SHIFT, i), time);
    i++;
    i%=orderCount;
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
}

void loop() {
  unsigned long currentTime = millis(); // Time since start
  unsigned long deltaTime = 0; // Time since last loop
  deltaTime = currentTime - lastTime;
  if (buttons[0].wasButtonPressed())
    stopwatches.startOrStopState();
  else if (buttons[1].wasButtonPressed())
    stopwatches.lappedState();
  else if (buttons[2].wasButtonPressed())
    stopwatches.resetTime();
  display.showNumber(stopwatches.returnTime(deltaTime));
  lastTime = currentTime;
 }
