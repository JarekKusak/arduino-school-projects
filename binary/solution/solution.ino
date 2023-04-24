#include "funshield.h"

constexpr int leds[] = {led1_pin, led2_pin, led3_pin, led4_pin};
constexpr int led_counter = sizeof(leds) / sizeof(leds[0]); // number of leds
constexpr int binary_digits = led_counter; // number of binary digits is dependent on led counter
constexpr int DELAY = 300;
constexpr int ONE_SECOND = 1000;
constexpr int MAX_NUMBER = led_counter << 2;
int binary_number[led_counter]; // array, which saves given number as ones and zeros
int numberToShow = 0;
unsigned long lastTime;

void toBinary(int n) {
  for (int i = binary_digits-1; i >= 0; i--) {
      binary_number[i]=n%2;
      n/=2;
  }
}

int calculateNumberToShow(int *numberToShow, bool inc_or_dec, int value) {
  *numberToShow = (inc_or_dec) ? (*numberToShow)+value : (*numberToShow)-value;
  if ((*numberToShow) < 0)
    *numberToShow = MAX_NUMBER-value; 
  *numberToShow=(*numberToShow)%MAX_NUMBER; // modulo 16
  return (*numberToShow);
}
      
void transferToBinaryAndDisplay(bool inc_or_dec, int *numberToShow, int value) {
  toBinary(calculateNumberToShow(numberToShow, inc_or_dec, value)); // transfers number to binary
  for (int i = 0; i < led_counter; i++){
    if(binary_number[i] == 1)
      digitalWrite(leds[i], ON);
    else
      digitalWrite(leds[i], OFF);
  }
}

class Button {
  private:
    int pin;
    bool Pressed;
    bool Was_pressed;
    bool Inc_or_dec; // true = increment, false = decrement
    bool one_second_passed;
    int one_second_timer;
    int timer;
    int increment_or_decrement_value;
  public:
    Button(int button_pin, byte is_pressed, byte was_pressed, bool inc_or_dec, int value) {
      pin = button_pin;
      Pressed = is_pressed;
      Was_pressed = was_pressed; 
      Inc_or_dec = inc_or_dec;
      one_second_passed = false;
      one_second_timer = 0;
      timer=0;
      increment_or_decrement_value = value;
  }

  void setupPin() {
    pinMode(pin, INPUT);
  }

  void resetvalues() { // pass by reference
    Was_pressed = OFF; // sets last button state as non-holded
    one_second_timer = 0; // resets one second timer
    one_second_passed = false; // sets as one second hasn't passed
    timer = 0; 
  }

  void wasButtonPressed(long deltaTime, int *numberToShow) {
    bool buttonState = digitalRead(pin);
    Pressed = buttonState;
    if (Pressed == ON){
      if (Was_pressed == OFF) // if button wasn't holded in previous iteration
        transferToBinaryAndDisplay(Inc_or_dec, numberToShow, increment_or_decrement_value); 
       
      Was_pressed = ON; // sets last button's state as holded
      one_second_timer+=deltaTime; // one second begins to be counted
      
      if (one_second_passed) // if one second passed after still holding the button, then start counting the second timer
        timer+=deltaTime;
      if (timer >= DELAY && one_second_passed){ // starts displaying binary numbers after holding the button for 1 second
        transferToBinaryAndDisplay(Inc_or_dec, numberToShow, increment_or_decrement_value);
        timer-=DELAY;
      }
      if (one_second_timer >= ONE_SECOND && !one_second_passed){  // if one second passed and one_second_passed hasn't been set
          one_second_passed = true;   
          transferToBinaryAndDisplay(Inc_or_dec, numberToShow, increment_or_decrement_value);
      }
    }
    else
      resetvalues(); // resets every value of button
  }
};

Button button1(button1_pin, OFF,OFF,true, 1);
Button button2(button2_pin, OFF,OFF,false, 1);
Button buttons[2] = {button1, button2};
constexpr int button_counter= sizeof(buttons) / sizeof(buttons[0]); // number of buttons 

void setup() {
  for (int i = 0; i < led_counter; i++) // init. of LEDs
    pinMode(leds[i], OUTPUT);
  for (int i = 0; i < button_counter; i++) // init. of LEDs
    buttons[i].setupPin();
  lastTime = millis(); // time at start
}

void loop() { // 1000 cycles/s
  unsigned long currentTime = millis(); // Time since start
  unsigned long deltaTime = 0; // Time since last loop
  deltaTime = currentTime - lastTime; 
  for (int i = 0; i < button_counter; i++)
    buttons[i].wasButtonPressed(deltaTime, &numberToShow); // pass by adress
  lastTime = currentTime;
}
