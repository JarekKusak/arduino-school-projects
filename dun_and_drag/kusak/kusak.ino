#include "funshield.h"

unsigned long timer = 0;
unsigned long lastTime;
bool previousMode = true; // mode before touching random button: true = normal, false = conf.
bool mode = true; // mode after pushing down the button: true = normal, false = conf.

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
      if (wasPressed == OFF)  // if button wasn't holded in previous iteration
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
    const int ones = 8; 
    const int tens = 4;
    const int hundreds = 2;
    const int thousands = 1;
    int index;
    byte d = 0b10100001; // letter d 
    int orderCount = sizeof(digit_muxpos) / sizeof(digit_muxpos[0]);
  public:
  
  void displayOutput(int order, int digit, bool mode) { 
    /*
    if (digit == 0 && order == hundreds && timer < 10000)
      return;
    if (digit == 0 && order == thousands && timer < 100000)
      return;
    */
    shiftOut(data_pin, clock_pin, MSBFIRST, (order == hundreds && !mode) ? digit : digits[digit]); // if order is at ones, show decimal point
    shiftOut(data_pin, clock_pin, MSBFIRST, order); // ... on positions 1 and 3 (0101)
    digitalWrite(latch_pin, LOW); // Trigger the latch
    digitalWrite(latch_pin, HIGH);
  }
  
  int getDigitAtPosition(long number, int position) {
    long multiplier = pow(10, position);
    return (number / multiplier) % 10;
  }

  void incrementIndex(){
    index++;
    index%=orderCount;
  }

  void showOnDisplay(int n, bool mode){
    displayOutput(digit_muxpos[orderCount-index-1], getDigitAtPosition(n, index), mode); // getDigitAtPosition(n, index)
    incrementIndex();
  }

  void showOnDisplayOneNumber(int n, bool mode) {
    displayOutput(digit_muxpos[orderCount-index-1], n, mode); // getDigitAtPosition(n, index)
    incrementIndex();
  }
  
  void displayConfigurationMode(int numberOfThrows, int diceType, bool mode) {
    if (digit_muxpos[orderCount-index-1] == hundreds) // show staticly letter d on hundreds order
      showOnDisplayOneNumber(d, mode);
    else if (digit_muxpos[orderCount-index-1] == thousands) // show staticly number of throws on thousand's place
      showOnDisplayOneNumber(numberOfThrows, mode); 
    else // on one's and ten's place show dice type (dice type of 100 is shown as 00)
      showOnDisplay(diceType, mode);
  }
  void displayNormalMode(int randomNumber, bool mode) {
    if (randomNumber > 0) {
      showOnDisplay(randomNumber, mode);   
    }    
  }
} display;

class Dice {
  private: 
    bool generatingSeed = false;
    int timer = 0;
    int numberOfThrows = 1;
    int maxNumberOfThrows = 10;
    int dices[7] = {4,6,8,10,12,20,100};
    int dicesCount = sizeof(dices)/sizeof(dices[0]); 
    int diceType = 0; 
    int randomNumber = 0;
  public:
  // getters
  int returnDiceType() { // getter for type of dice
    return dices[diceType];
  }
  int returnNumberOfThrows(){ // getter for number of throws
    return numberOfThrows;
  }
  int returnGeneratedRandomNumber(){
    return randomNumber;
  }
  
  void increaseTimer(long deltaTime){
    generatingSeed = true;
    timer += deltaTime;
  }
  
  int checkIfSeedGenerated() {
    if (!generatingSeed && timer > 0) { // if generating seed already stopped and timer is non-zero, then generate random number
      randomSeed(timer);
      randomNumber = numberOfThrows*random(1,dices[diceType]); // 1 to 4 chance
      Serial.println(timer);
      Serial.println(randomNumber);
      timer = 0;
    }
    generatingSeed = false;
    return randomNumber;
  }

  void increaseNumberOfThrows(int value){
    numberOfThrows+=value;
    if (numberOfThrows == maxNumberOfThrows)
      numberOfThrows = 1;
    Serial.println(numberOfThrows);
  }

  void changeDiceType(){
    diceType++;
    diceType%=dicesCount;
    Serial.println(dices[diceType]);
  }

} dice;

Button button1(button1_pin); // normal mode (generates random seed)
Button button2(button2_pin, 1); // conf. mode (increases number of throws)
Button button3(button3_pin); // conf. mode (increases sides of dice)
Button buttons[3] = {button1, button2, button3};
constexpr int buttonsCount = sizeof(buttons) / sizeof(buttons[0]);

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

constexpr int DELAY = 200;
void loop() {
  unsigned long currentTime = millis(); // Time since start
  unsigned long deltaTime = currentTime - lastTime; // Time since last loop
  bool seedGeneratingButtonBeingHeld = (buttons[0].isButtonBeingHeld() == ON) ? true : false;
  if (seedGeneratingButtonBeingHeld) {
    mode = true;
    if (previousMode)
      dice.increaseTimer(deltaTime);      
  }
  else if (buttons[1].wasButtonPressed()) { // button that increments number of throws (conf. mode)
    mode = false;
    //(previousMode == mode) ? dice.increaseNumberOfThrows() : previousMode = mode; 
    if (previousMode == mode) {
      dice.increaseNumberOfThrows(buttons[1].buttonValue());
      Serial.println("přičítáme počet hodů");
    } 
    else {
      previousMode = mode;
      Serial.println("mění se mode v 2");
    }
  }
  else if (buttons[2].wasButtonPressed()) { // button that switches orders (conf. mode)
    mode = false;
    //(previousMode == mode) ? dice.changeDiceType() : (previousMode = mode);
    if (previousMode == mode) {
      dice.changeDiceType();
    } 
    else {
      previousMode = mode;
    }
  }
  if (previousMode != mode && !seedGeneratingButtonBeingHeld) // if previous mode isn't actual and seed generating button is not being held anymore, then set present mode
    previousMode = mode;
  (mode) ? display.displayNormalMode(dice.checkIfSeedGenerated(), mode) : display.displayConfigurationMode(dice.returnNumberOfThrows(), dice.returnDiceType(), mode);
  lastTime = currentTime;
}
