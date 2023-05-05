#include "funshield.h"

unsigned long lastTime;
bool previousMode = true; // mode before touching random button: true = normal, false = conf.
bool mode = true; // mode after pushing down the button: true = normal, false = conf.
bool generating = false; // detects if generating seed right now

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
  int buttonValue() { // getter for value of button (for instance incrementation value)
    return value;
  }
  void setupPin() {
    pinMode(pin, INPUT);
  }
  // can detect a longer button hold
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
    const int DELAY = 100;
    const int leds[4] = {led1_pin, led2_pin, led3_pin, led4_pin};
    const int ledCounter = sizeof(leds)/sizeof(leds[0]);
    unsigned int LED_number = 0;
    unsigned int LED_to_turn_off = 1;
    bool direction;
    int index = 0;
    // totally random characters for simple animation
    byte randomChar1 = 0b10101000; 
    byte randomChar2 = 0b10010011;
    byte randomChar3 = 0b11000110;
 
    byte d = 0b10100001; // letter d 
    int animation[3]={randomChar1, randomChar2, randomChar3};
    int animationCharactersCount = sizeof(animation)/sizeof(animation[0]);
    int orderCount = sizeof(digit_muxpos) / sizeof(digit_muxpos[0]);
    int animationLetter = 0;
    int animationTimer = 0;
  public:
  
  void displayOutput(int order, int digit, bool mode) { 
    shiftOut(data_pin, clock_pin, MSBFIRST, (order == hundreds && !mode) ? digit : digits[digit]); // if order is at hundreds at conf. mode, show letter
    shiftOut(data_pin, clock_pin, MSBFIRST, order); // ... on positions 1 and 3 (0101)
    digitalWrite(latch_pin, LOW); // Trigger the latch
    digitalWrite(latch_pin, HIGH);
  }

  void turningLedsOff(){
    for (int i = 0; i < ledCounter; i++)
      digitalWrite(leds[i], OFF); 
  }

  void turningLedsOnAndOff(int LED_turn_on_index, int LED_turn_off_index) {
    digitalWrite(leds[LED_turn_on_index], ON);
    digitalWrite(leds[LED_turn_off_index], OFF);  
  }

  bool setLedsAndReturnDirection(int LED_turn_on_index, int LED_turn_off_index, bool direction) { // function sets LEDs and returns the direction of "blinking"
    turningLedsOnAndOff(LED_turn_on_index, LED_turn_off_index);
    if (LED_turn_on_index == 0)
      direction = true;
    else if (LED_turn_on_index == 3)
      direction = false;
    return direction;
  }
  
  int getDigitAtPosition(long number, int position) {
    long multiplier = pow(10, position);
    return (number / multiplier) % 10;
  }

  void incrementIndex() {
    index++;
    index%=orderCount;
  }

  void showOnDisplay(int n, bool mode, bool multiDigitNumber) {
    displayOutput(digit_muxpos[orderCount-index-1], (multiDigitNumber) ? getDigitAtPosition(n, index) : n, mode); // getDigitAtPosition(n, index)
    incrementIndex();
  }

  void showWhileGenerating(bool mode, long deltaTime) {
    showOnDisplay(animation[animationLetter], mode, false);
    animationTimer += deltaTime;
    if (animationTimer >= DELAY){
      direction = setLedsAndReturnDirection(LED_number, LED_to_turn_off, direction);
      int increment_or_decrement = (direction) ? 1 : -1; // sets +1 or -1 due to the direction
      LED_to_turn_off = LED_number; // sets actual LED as the one to turn off next iteration
      LED_number = LED_number + increment_or_decrement; // increment or decrement by auxiliary variable
      animationLetter++;
      animationTimer = 0;
    }   
    animationLetter%=animationCharactersCount;
  }
  
  void displayConfigurationMode(int numberOfThrows, int diceType, bool mode) {
    if (digit_muxpos[orderCount-index-1] == hundreds) // show staticly letter "d" on hundreds order
      showOnDisplay(d, mode, false);
    else if (digit_muxpos[orderCount-index-1] == thousands) // show staticly number of throws on thousand's place
      showOnDisplay(numberOfThrows, mode, false); 
    else // on one's and ten's place show dice type (dice type of 100 is shown as 00)
      showOnDisplay(diceType, mode, true);
  }
  
  void displayNormalMode(int randomNumber, bool mode, bool generating) {
    if (!generating) {
        showOnDisplay(randomNumber, mode, true);
        turningLedsOff();
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
  int returnNumberOfThrows() { // getter for number of throws
    return numberOfThrows;
  }
  int returnGeneratedRandomNumber() { // getter for randomNumber
    return randomNumber;
  }
  
  void increaseTimer(long deltaTime) {
    generatingSeed = true;
    timer += deltaTime;
  }
  
  int checkIfSeedGenerated() {
    if (!generatingSeed && timer > 0) { // if generating seed already stopped and timer is non-zero, then generate random number
      randomSeed(timer); // generates a random seed depending on the elapsed time
      int sumOfRandomNumbers = 0;
      for (int i = 0; i < numberOfThrows; i++)
        sumOfRandomNumbers += random(1,dices[diceType]); 
      randomNumber = sumOfRandomNumbers;
      timer = 0;
    }
    generatingSeed = false;
    return randomNumber;
  }

  void increaseNumberOfThrows(int value) {
    numberOfThrows+=value;
    if (numberOfThrows == maxNumberOfThrows)
      numberOfThrows = 1;
  }

  void changeDiceType(){
    diceType++;
    diceType%=dicesCount;
  }

} dice;

Button button1(button1_pin); // normal mode (generates random seed)
Button button2(button2_pin, 1); // conf. mode (increases number of throws - by one)
Button button3(button3_pin); // conf. mode (increases sides of dice)
Button buttons[3] = {button1, button2, button3};
constexpr int buttonsCount = sizeof(buttons) / sizeof(buttons[0]);

void setup() {
  lastTime = millis(); // time at start
  for (int i = 0; i < buttonsCount; i++)
    buttons[i].setupPin();
   // init. of LEDs (had to do it without cycle, because all variables for leds are in class Display)
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);
  pinMode(led3_pin, OUTPUT);
  pinMode(led4_pin, OUTPUT);
  
  randomSeed(analogRead(0)); // initialize randomSeed
  
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  display.turningLedsOff();
}

void loop() {
  unsigned long currentTime = millis(); // Time since start
  unsigned long deltaTime = currentTime - lastTime; // Time since last loop
  bool seedGeneratingButtonBeingHeld = (buttons[0].isButtonBeingHeld() == ON) ? true : false;
  if (seedGeneratingButtonBeingHeld) {
    mode = true; // switches mode to normal mode (true)
    if (previousMode) { // if previous mode was configuration mode, then don't do anything until user unholds the button (we want only to switch into normal mode, not immediately generate)
      display.showWhileGenerating(mode, deltaTime);
      generating = true;
      dice.increaseTimer(deltaTime);
    }      
  }
  else if (buttons[1].wasButtonPressed()) { // button that increments number of throws (conf. mode)
    mode = false; // switches mode to configuration mode (false)
    //(previousMode == mode) ? dice.increaseNumberOfThrows(buttons[1].buttonValue()) : previousMode = mode; // cannot be written as ternary operator ???
    if (previousMode == mode) 
      dice.increaseNumberOfThrows(buttons[1].buttonValue());
    else 
      previousMode = mode;
  }
  else if (buttons[2].wasButtonPressed()) { // button that switches orders (conf. mode)
    mode = false; // switches mode to configuration mode (false)
    //(previousMode == mode) ? dice.changeDiceType() : (previousMode = mode); // cannot be written as ternary operator ???
    if (previousMode == mode) 
      dice.changeDiceType();   
    else 
      previousMode = mode;    
  }
  if (previousMode != mode && !seedGeneratingButtonBeingHeld) // if previous mode isn't actual and seed generating button is not being held anymore, then set current mode
    previousMode = mode;
  (mode) ? display.displayNormalMode(dice.checkIfSeedGenerated(), mode, generating) : display.displayConfigurationMode(dice.returnNumberOfThrows(), dice.returnDiceType(), mode); // display depends on current mode
  generating = false;
  lastTime = currentTime;
}
