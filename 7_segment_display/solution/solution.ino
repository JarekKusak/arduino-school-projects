#include "funshield.h"

int number=0;
// order is represented by power of two's (bcs of 7-segment-display protocol)
// we have 4 digits to display 
int orderToShow[4] = {1,2,4,8};
constexpr int orderCount = sizeof(orderToShow) / sizeof(orderToShow[0]);
// !! order is upside down, the highest power of two is the least order !!
int order = orderCount-1; // sets as first order (otherway around)
constexpr int MAX_NUMBER = 10000; // upper barrier

class Button {
  public:
    int Pin;
    bool IncOrDec; // true = increment, false = decrement/nothing
    bool Switch_button;
    byte Pressed;
    byte WasPressed;
    
  Button(int button_pin, bool inc_or_dec, bool does_switch) {
    Pin = button_pin;
    IncOrDec = inc_or_dec;
    Switch_button = does_switch;
    Pressed = OFF;
    WasPressed = OFF;
  }

  void setupPin() {
    pinMode(Pin, INPUT);
  }
};

Button button1(button1_pin, true, false);
Button button2(button2_pin, false, false);
Button button3(button3_pin, false, true); // button that switches into different power of ten
Button buttons[3] = {button1, button2, button3};
constexpr int buttonsCount = sizeof(buttons) / sizeof(buttons[0]);

int returnDigit(int number){
  int i = 0;
  int remainder;

  // if the showed order is higher than actul number, then just return 0 as digit to show
  for (int j = 0; j < orderCount; j++)
  {
      if (number < pow(10,j+1) && orderCount-order-1 > j)
          return 0;
  }

  // getting digits out of the number (finding the digit to show on the display)
  while (number != 0) {
      remainder = number % 10;
      if (i == orderCount-order-1) // if the digit is the one to show, then just return it
        return remainder; // the searched digit
      i++;
      number = number / 10;
  }
  return 0;
}

void displayOutput(int order, int digit) {
  shiftOut(data_pin, clock_pin, MSBFIRST, digits[digit]);   // Display digit 7 (11111000) ...
  shiftOut(data_pin, clock_pin, MSBFIRST, order);     // ... on positions 1 and 3 (0101)
  digitalWrite(latch_pin, LOW);                   // Trigger the latch
  digitalWrite(latch_pin, HIGH);
}

void setup() {
  for (int i = 0; i < buttonsCount; i++) {
    buttons[i].setupPin();
  }
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  displayOutput(orderToShow[order], number); // displays zero
}
  
void action(Button& button) {
  if (button.Switch_button) {
    order--; // smaller value of order = higher actual order of number
    if (order < 0) 
      order=orderCount-1; // underflow (order overflow) - just sets as the first order
  }
  else if (button.IncOrDec) { // true == increment
    number=number+pow(10,orderCount-order-1); // add's current order's power of ten
    number=number%MAX_NUMBER;
  }
  else { // decrement
    number=number-pow(10,orderCount-order-1); // substract's current order's power of ten
    if (number < 0) // if underflow, substract max number and the negative value
      number=MAX_NUMBER+number;
  }
  displayOutput(orderToShow[order], returnDigit(number)); // outputs on display, two parameters: order and the digit of number
}

void wasButtonPressed(Button& button) {
  byte buttonState = digitalRead(button.Pin);
  button.Pressed = buttonState;
  if (button.Pressed == ON) {
    if (button.WasPressed == OFF) // if button wasn't holded in previous iteration
      action(button);
    button.WasPressed = ON; // sets last button's state as held
  }
  else
     button.WasPressed = OFF;
}

void loop() {
  for (int i = 0; i < buttonsCount; i++)
    wasButtonPressed(buttons[i]); 
}
