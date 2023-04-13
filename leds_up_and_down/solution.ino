//#include <funshield.h>

constexpr int leds[] = {led1_pin, led2_pin, led3_pin, led4_pin};
unsigned long lastTime;
unsigned long timer;
unsigned constexpr int DELAY = 300;
unsigned int LED_number;
unsigned int LED_to_turn_off;
bool direction;

void setup() {
  // init. of LEDs
  for (int i = 0; i < 4; i++)
  {
    pinMode(leds[i], OUTPUT);
  }
  lastTime = millis(); // time at start
  timer = 0;
  LED_number = 0; // starting from first LED (indexed from 0)
  LED_to_turn_off = 1; // turning off second LED (just for initialization)
  direction = true; // true = from left to right, false = from right to left
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

void loop() { // 1000 cycles/s
  unsigned long currentTime = millis();               // Time since start
  unsigned long deltaTime = currentTime - lastTime;   // Time since last loop
  if (timer >= DELAY){ // 300ms = constant
    timer = timer - DELAY;
    direction = setLedsAndReturnDirection(LED_number, LED_to_turn_off, direction);
    int increment_or_decrement = (direction) ? 1 : -1; // sets +1 or -1 due to the direction
    LED_to_turn_off = LED_number; // sets actual LED as the one to turn off next iteration
    LED_number = LED_number + increment_or_decrement; // increment or decrement by auxiliary variable
  }
  lastTime = currentTime;
  timer += deltaTime;   
}
