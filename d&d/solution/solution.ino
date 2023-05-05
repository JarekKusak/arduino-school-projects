#include "funshield.h"

unsigned long lastTime;

class Button {
  private:
    int pin;
    int value;
    bool pressed;
    bool wasPressed;
  public:
  Button(int button_pin, int button_value = 0) {
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
  // can detect longer holding the button
  bool isButtonBeingHeld() {
    bool buttonState = digitalRead(pin);
    return buttonState;
  }
  // discrete pushing the button
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
    byte d = 0b10100001;
    const int ONE_SECOND = 1000;
    int orderCount = sizeof(digit_muxpos) / sizeof(digit_muxpos[0]);
    bool start = false;
  public:
  
  void displayOutput(int order, int digit) {
    shiftOut(data_pin, clock_pin, MSBFIRST, (order == ones) ? digits[digit]-128 : digits[digit]); // if order is at ones, show decimal point
    shiftOut(data_pin, clock_pin, MSBFIRST, order); // ... on positions 1 and 3 (0101)
    digitalWrite(latch_pin, LOW); // Trigger the latch
    digitalWrite(latch_pin, HIGH);
  }
  
} display;

class Dice {
  private: 
    int timer = 0;
    bool generatingSeed = false;
    int dices[7] = {4,6,8,10,12,20,100};
    int dicesCount = 
    int diceType = 0;
  public:
    
    void increaseTimer(long deltaTime){
      generatingSeed = true;
      timer += deltaTime;
    }
    
    int checkIfSeedGenerated(){
      int randomNumber = 0;
      if (!generatingSeed && timer > 0) {
        randomSeed(timer);
        randomNumber = random(1,dices[diceType]); // 1 to 4 chance
        timer = 0;
      }
      generatingSeed = false;
      return randomNumber;
    }

    int changeDiceType(){
      diceType++;
      diceType%
    }

} dice;

Button button1(button1_pin); // normal mode (generates random seed)
Button button2(button2_pin, 1); // conf. mode (increases number of throws)
Button button3(button3_pin); // conf. mode (increases sides of dice)
Button buttons[3] = {button1, button2, button3};
constexpr int buttonsCount = sizeof(buttons) / sizeof(buttons[0]);
unsigned long timer = 0;

void setup() {
  lastTime = millis(); // time at start
  for (int i = 0; i < buttonsCount; i++)
    buttons[i].setupPin();
  randomSeed(analogRead(0)); // inicialize randomSeed
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long currentTime = millis(); // Time since start
  unsigned long deltaTime = currentTime - lastTime; // Time since last loop
  if (buttons[0].isButtonBeingHeld() == ON)
    dice.increaseTimer(deltaTime);
  else if (buttons[1].wasButtonPressed()){ // button that increments number of throws (conf. mode)
    
  }
  else if (buttons[2].wasButtonPressed()){ // button that switches orders (conf. mode)
  }
  if (dice.checkIfSeedGenerated() > 0)
    Serial.println("jj funguje");
  lastTime = currentTime;
}
