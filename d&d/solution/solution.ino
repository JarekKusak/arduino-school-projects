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
    byte d = 0b10100001;
    int orderCount = sizeof(digit_muxpos) / sizeof(digit_muxpos[0]);
    bool configurationMode = false;
  public:
  
  void displayOutput(int order, int digit) { 
    /*
    if (digit == 0 && order == hundreds && timer < 10000)
      return;
    if (digit == 0 && order == thousands && timer < 100000)
      return;
    */
    shiftOut(data_pin, clock_pin, MSBFIRST, (order == hundreds) ? digit : digits[digit]); // if order is at ones, show decimal point
    shiftOut(data_pin, clock_pin, MSBFIRST, order); // ... on positions 1 and 3 (0101)
    digitalWrite(latch_pin, LOW); // Trigger the latch
    digitalWrite(latch_pin, HIGH);
  }
  
  int getDigitAtPosition(long number, int position) {
    long multiplier = pow(10, position);
    return (number / multiplier) % 10;
  }

  void showOnDisplay(int n){
    displayOutput(digit_muxpos[orderCount-index-1], getDigitAtPosition(n, index)); // getDigitAtPosition(n, index)
    index++;
    index%=orderCount;
  }

  void showOnDisplayOneNumber(int n) {
    displayOutput(digit_muxpos[orderCount-index-1], n); // getDigitAtPosition(n, index)
    index++;
    index%=orderCount;
  }
  
  void displayConfigurationMode(int numberOfThrows, int diceType) {
    //Serial.println("jojo");
    if (digit_muxpos[orderCount-index-1] == ones)
      showOnDisplay(diceType); 
    else if (digit_muxpos[orderCount-index-1] == tens)
      showOnDisplay(diceType);
    else if (digit_muxpos[orderCount-index-1] == hundreds)
      showOnDisplayOneNumber(d);
    else if (digit_muxpos[orderCount-index-1] == thousands)
      showOnDisplayOneNumber(numberOfThrows); 
      
  }
  void displayNormalMode(int randomNumber) {
    if (randomNumber > 0) {
      showOnDisplay(randomNumber);   
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

  void increaseNumberOfThrows(){
    numberOfThrows++;
    numberOfThrows%=maxNumberOfThrows;
    Serial.println(numberOfThrows);
  }

  void changeDiceType(){
    diceType++;
    diceType%=dicesCount;
    Serial.println(dices[diceType]);
  }

} dice;

Button button1(button1_pin, 0); // normal mode (generates random seed)
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
bool previousMode = true; // mode before touching random button: true = normal, false = conf.
bool mode = true; // mode after pushing down the button: true = normal, false = conf.

void loop() {
  unsigned long currentTime = millis(); // Time since start
  unsigned long deltaTime = currentTime - lastTime; // Time since last loop
  if (!mode && buttons[0].wasButtonPressed()) {
    mode = true;
    previousMode = mode;
    Serial.println("mění se mode v 1");  
  }
  else if (buttons[0].isButtonBeingHeld() == ON){
    dice.increaseTimer(deltaTime);
  }
  /*if (buttons[0].wasButtonPressed()){
    mode = true;
    if (previousMode == mode) {
      dice.increaseTimer(deltaTime);
      Serial.println("generujeme");
    } 
    else {
      previousMode = mode;
      Serial.println("mění se mode v 1");
    }
  
  }*/
  else if (buttons[1].wasButtonPressed()){ // button that increments number of throws (conf. mode)
    mode = false;
    //(previousMode == mode) ? dice.increaseNumberOfThrows() : previousMode = mode; 
    if (previousMode == mode) {
      dice.increaseNumberOfThrows();
      Serial.println("přičítáme počet hodů");
    } 
    else {
      previousMode = mode;
      Serial.println("mění se mode v 2");
    }
    
  }
  else if (buttons[2].wasButtonPressed()){ // button that switches orders (conf. mode)
    mode = false;
    //(previousMode == mode) ? dice.changeDiceType() : previousMode = mode;
    if (previousMode == mode) {
      dice.changeDiceType();
    } 
    else {
      previousMode = mode;
    }
  }
  if (mode)
    display.displayNormalMode(dice.checkIfSeedGenerated());
  else
    display.displayConfigurationMode(dice.returnNumberOfThrows(), dice.returnDiceType());
  lastTime = currentTime;
}
