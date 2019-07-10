#include "Arduino.h"

#define LED_BLINK_INTERVAL 100

bool ledLightStatus = false;

void LED_turnOn(){
  digitalWrite(LED_BUILTIN, HIGH);
  ledLightStatus = true;
}

void LED_turnOff(){
  digitalWrite(LED_BUILTIN, LOW);
  ledLightStatus = false;
}

void LED_blink(int blinkCount){
  for(int i=0;i<blinkCount;i++){
    if (ledLightStatus){
      LED_turnOff();
      delay(LED_BLINK_INTERVAL);
      LED_turnOn();
    } else {
      LED_turnOn();
      delay(LED_BLINK_INTERVAL);
      LED_turnOff();
    }
    delay(LED_BLINK_INTERVAL);
  }
}

void LED_init(){
  pinMode(LED_BUILTIN, OUTPUT);
  LED_turnOff();
  LED_blink(1);
}
