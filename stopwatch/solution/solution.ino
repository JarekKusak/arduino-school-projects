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
    const int ONE_SECOND = 1000; // unused variable, cannot be multiplied by 10 nor 100 in conditions below..
    long timer = 0;
    int orderCount = sizeof(digit_muxpos) / sizeof(digit_muxpos[0]);
    int i = 0;
    bool start = false;
    bool show = true;
    long lastSavedTime;
  public:
  void displayOutput(int order, int digit) {
    if (digit == 0 && order == tens && timer < 10000) // if the digit to show is zero, order is at tens and timer (elapsed time) is less than 10 seconds, then do not show anything 
      return;
    if (digit == 0 && order == hundreds && timer < 100000) // if the digit to show is zero, order is at hundreds and timer (elapsed time) is less than 100 seconds, then do not show anything 
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
  
  void showNumber(bool firstButtonWasPressed, bool secondButtonWasPressed, bool thirdButtonWasPressed, int deltaTime) {
    
    if (firstButtonWasPressed && !start) // starting the count
      start = true;
    else if (firstButtonWasPressed && start && show) // stoping the count (also checking if not in lapped state)
      start = false;

    if (start && secondButtonWasPressed && show) // switching into lapped state (still counting but not showing)
       show = false;
    else if (start && secondButtonWasPressed && !show) // back into non-lapped mode
       show = true;
    
    if (!start && thirdButtonWasPressed) // reseting the button if not running
      timer = 0;
        
    if (start)
      startCounting(deltaTime); 
    
    if (show) { // showing the elapsed time
      lastSavedTime = timer;
      displayOutput(digit_muxpos[orderCount-i-1], getDigitAtPosition(timer/SHIFT, i));
    }
    else // not showing the elapsed time (showing last saved time)
      displayOutput(digit_muxpos[orderCount-i-1], getDigitAtPosition(lastSavedTime/SHIFT, i));
         
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
}

void loop() {
  unsigned long currentTime = millis(); // Time since start
  unsigned long deltaTime = 0; // Time since last loop
  deltaTime = currentTime - lastTime;
  display.showNumber(buttons[0].wasButtonPressed(), buttons[1].wasButtonPressed(), buttons[2].wasButtonPressed(), deltaTime);
  lastTime = currentTime;
}
