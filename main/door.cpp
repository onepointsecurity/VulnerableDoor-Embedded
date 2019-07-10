#include "Arduino.h"
#include "serial.h"
#include "ble.h"
#include "common.h"
#include "debug.h"
#include "led.h"

#define SETUP_PROCEDURE_TIME_SECONDS 10 
#define SETUP_PROCEDURE_OPENING_ITERATIONS 5
#define DOOR_SENSOR_PIN 2 //ESP8266 RX on DVID

unsigned long firstDoorOpenMillis = 0;
int doorOpenCount = 0;
bool doorStatusOpen = false;
unsigned long timeToTriggerScan = 0;

void door_open(){
  serial_println("Door Opened");
  if(doorOpenCount > 0 && millis() < firstDoorOpenMillis + SETUP_PROCEDURE_TIME_SECONDS * 1000){
    // Increment if the door is reopened less than 10s before first aperture
    doorOpenCount ++;
  } else {
    doorOpenCount = 1;
    firstDoorOpenMillis = millis();
  }

  if (doorOpenCount >= SETUP_PROCEDURE_OPENING_ITERATIONS && getMode() != MODE_CONFIG){
    // CONFIG Mode Sequence
    serial_println("Entering config mode due to door open/close sequence.");
    doorOpenCount = 0;
    setMode(MODE_CONFIG);
  } else if (getMode() == MODE_NORMAL){
    // Just a regular Door opening
    // Arm a trigger
    timeToTriggerScan = millis() + SETUP_PROCEDURE_TIME_SECONDS * 1000;
    serial_println(F("Arming presence check trigger"));
  }
  delay(500); // Debounce
}

void door_close(){
  serial_println("Door Closed");
  if (getMode() == MODE_CONFIG && configValid()){
    serial_println("Leaving setup mode.");
    doorOpenCount = 0;
    setMode(MODE_NORMAL);
  }
  delay(500); // Debounce
}

void door_init(){
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  doorStatusOpen = digitalRead(DOOR_SENSOR_PIN);
}

void door_check(){
  if (millis() > timeToTriggerScan && getMode() == MODE_NORMAL && doorOpenCount > 0){
    LED_blink(3);
    doorOpenCount = 0;

    if (not ble_isConnected()){
      triggerAlarm();
    } else {
      serial_println(F("Device is connected so alarm is inhibited"));
    }
  }
  
  bool oldDoorStatusOpen = doorStatusOpen;
  doorStatusOpen = digitalRead(DOOR_SENSOR_PIN);
  if (not oldDoorStatusOpen == doorStatusOpen){
    // Door status changed
    if (doorStatusOpen){
      door_open();
    } else {
      door_close();
    }
  }
}
