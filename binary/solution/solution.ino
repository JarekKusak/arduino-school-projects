#include <funshield.h>

constexpr int leds[] = {led1_pin, led2_pin, led3_pin, led4_pin};
constexpr int led_counter = sizeof(leds) / sizeof(leds[0]); // number of leds
constexpr int binary_digits = led_counter; // number of binary digits is dependent on led counter
constexpr int DELAY = 300;
constexpr int ONE_SECOND = 1000;
constexpr int MAX_NUMBER = 16;
int binary_number[led_counter]; // array, which saves given number as ones and zeros
int number_to_show = 0;
unsigned long lastTime;

class Button {
public:
  int pin;
  byte Pressed;
  byte Was_pressed;
  bool Inc_or_dec; // true = increment, false = decrement
  bool One_second_passed;
  int One_second_timer;
  int timer;

  Button(int button_pin, byte is_pressed, byte was_pressed, bool inc_or_dec) {
    pin = button_pin;
    Pressed = is_pressed;
    Was_pressed = was_pressed; 
    Inc_or_dec = inc_or_dec;
    One_second_passed = false;
    One_second_timer = 0;
    timer=0;
  }

  void setupPin(){
    pinMode(pin, INPUT);
  }
};

void toBinary(int n){
  for (int i = binary_digits-1; i >= 0; i--){
      binary_number[i]=n%2;
      n/=2;
    }
}
      
void displayBinary(bool inc_or_dec) {
  number_to_show = (inc_or_dec) ? number_to_show+1 : number_to_show-1;
  if (number_to_show < 0)
    number_to_show = MAX_NUMBER-1; // set as MAX_NUMBER-1 (only ones in binary)
  number_to_show=number_to_show%MAX_NUMBER; // modulo 16
  toBinary(number_to_show); // transfers number to binary
  for (int i = 0; i < led_counter; i++){
    if(binary_number[i] == 1)
      digitalWrite(leds[i], ON);
    else
      digitalWrite(leds[i], OFF);
  }
}

Button button1(button1_pin, OFF,OFF,true);
Button button2(button2_pin, OFF,OFF,false);
Button buttons[2] = {button1, button2};
constexpr int button_counter= sizeof(buttons) / sizeof(buttons[0]); // number of buttons 

void setup() {
  for (int i = 0; i < led_counter; i++) // init. of LEDs
    pinMode(leds[i], OUTPUT);
  for (int i = 0; i < button_counter; i++) // init. of LEDs
    buttons[i].setupPin();
  lastTime = millis(); // time at start
}

void resetValues(Button &button){ // pass by reference
  button.Was_pressed = OFF; // sets last button state as non-holded
  button.One_second_timer = 0; // resets one second timer
  button.One_second_passed = false; // sets as one second hasn't passed
  button.timer = 0; 
}

void wasButtonPressed(Button &button, long deltaTime){
  byte button_state = digitalRead(button.pin);
  button.Pressed = button_state;
  if (button.Pressed == ON){
    if (button.Was_pressed == OFF) // if button wasn't holded in previous iteration
      displayBinary(button.Inc_or_dec); 
     
    button.Was_pressed = ON; // sets last button's state as holded
    button.One_second_timer+=deltaTime; // one second begins to be counted
    
    if (button.One_second_passed) // if one second passed after still holding the button, then start counting the second timer
      button.timer+=deltaTime;
    if (button.timer >= DELAY && button.One_second_passed){ // starts displaying binary numbers after holding the button for 1 second
      displayBinary(button.Inc_or_dec);
      button.timer-=DELAY;
    }
    if (button.One_second_timer >= ONE_SECOND && !button.One_second_passed){  // if one second passed and One_second_passed hasn't been set
        button.One_second_passed = true;   
        displayBinary(button.Inc_or_dec);
    }
  }
  else
    resetValues(button); // resets every value of button
}

void loop() { // 1000 cycles/s
  unsigned long currentTime = millis(); // Time since start
  unsigned long deltaTime = 0; // Time since last loop
  deltaTime = currentTime - lastTime; 
  for (int i = 0; i < button_counter; i++)
    wasButtonPressed(buttons[i], deltaTime); 
  lastTime = currentTime;
}
