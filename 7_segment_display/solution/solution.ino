#include "funshield.h"

int number=0;
// order is represented by power of two's (bcs of 7-segment-display protocol)
// we have 4 digits to display 
int orderToShow[4] = {1,2,4,8};
constexpr int orderCount = sizeof(orderToShow) / sizeof(orderToShow[0]);
// !! order is upside down, the highest power of two is the least order !!
int order = orderCount-1; // sets as first order (otherway around)
constexpr int MAX_NUMBER = 10000; // upper barrier

int returnDigit(int number) {
  int i = 0;
  int remainder;

  // if the showed order is higher than actual number, then just return 0 as digit to show
  for (int j = 0; j < orderCount; j++) {
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
  shiftOut(data_pin, clock_pin, MSBFIRST, digits[digit]); // Display digit 7 (11111000) ...
  shiftOut(data_pin, clock_pin, MSBFIRST, order); // ... on positions 1 and 3 (0101)
  digitalWrite(latch_pin, LOW); // Trigger the latch
  digitalWrite(latch_pin, HIGH);
}

class Button {
  private:
    int pin;
    int value;
    bool pressed;
    bool wasPressed;
  public:
  Button(int button_pin, int button_value = 1) {
    pin = button_pin;
    value = button_value;
    pressed = OFF;
    wasPressed = OFF;
  }

  int buttonValue() { // gets value for incrementation or decrementation
    return value;
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

Button button1(button1_pin); // increment button (default value for increment is one)
Button button2(button2_pin, -1); // decrement button (default value for decrement is minus one)
Button button3(button3_pin); // button that switches into different power of ten
Button buttons[3] = {button1, button2, button3};
constexpr int buttonsCount = sizeof(buttons) / sizeof(buttons[0]);

void setup() {
  for (int i = 0; i < buttonsCount; i++)
    buttons[i].setupPin();
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  displayOutput(orderToShow[order], number); // displays zero as default
}

int increment_or_decrement(int number, int value){
  number=number+value*pow(10,orderCount-order-1); // add's current order's power of ten
  if (number < 0) // if underflow, substract max number and the negative value
    number=MAX_NUMBER+number;
  number=number%MAX_NUMBER;
  return number;
}

int switchOrder(int order){
  order--; // smaller value of order = higher actual order of number
  if (order < 0) 
    order=orderCount-1; // underflow (order overflow) - just sets as the first order
  return order;
}

void loop() {
  if(buttons[0].wasButtonPressed()) { // button that increments number
    number = increment_or_decrement(number, buttons[0].buttonValue());
    displayOutput(orderToShow[order], returnDigit(number)); // outputs on display, two parameters: order and the digit of number
  }
  else if (buttons[1].wasButtonPressed()){ // button that decrements number
    number = increment_or_decrement(number, buttons[1].buttonValue()); 
    displayOutput(orderToShow[order], returnDigit(number)); // outputs on display, two parameters: order and the digit of number   
  }
  else if (buttons[2].wasButtonPressed()){ // button that switches orders
    order = switchOrder(order); 
    displayOutput(orderToShow[order], returnDigit(number)); // outputs on display, two parameters: order and the digit of number    
  }
}
