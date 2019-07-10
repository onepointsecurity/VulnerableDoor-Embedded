#include "Arduino.h"
#include "debug.h"
#include "serial.h"
#include "ble.h"
#include "common.h"
#include "led.h"
#include "gsm.h"

int mode = MODE_INIT;
String CFG_phoneNumber = "0000000000";

void setMode(int newMode){
  switch(newMode){
    case MODE_INIT:
      serial_println(F("==========[MODE INIT]=========="));
      mode = newMode;
      LED_blink(10);
      break;
    case MODE_NORMAL:
      serial_println(F("==========[MODE NORMAL]=========="));
      mode = newMode;
      LED_turnOff();  
      ble_setMode(MODE_NORMAL);
      break;
    case MODE_CONFIG:
      serial_println(F("==========[MODE CONFIG]=========="));
      mode = newMode;
      LED_turnOn();
      ble_setMode(MODE_CONFIG);
      break;
    default:
      debug_println(F("ERROR: Unknown Mode"));
      break;
  }
}

int getMode(){
  return mode;
}

bool configValid(){
  if (not CFG_phoneNumber.equals("0000000000") && ble_isMACConfigured()){
    return true;
  } else {
    return false;
  }
}

void savePhoneNumber(const String& phoneNumber){
  CFG_phoneNumber = phoneNumber;
  serial_print("Phone Number Configured: ");
  serial_println((char*)phoneNumber.c_str()); 
  //String plaintext = String("OK");
  //String cyphertext = ble_encrypt(plaintext);
  //ble_println(cyphertext);
  ble_send_ciphered("OK");
}

String getPhoneNumber(){
  return CFG_phoneNumber;
}

void triggerAlarm(){
    serial_println(F("!!!!!!!!!!!!!![Alarm triggered]!!!!!!!!!!!!!!!!!!"));
    serial_print(F("SMS sent to "));
    serial_println(CFG_phoneNumber);
    gsm_sendsms("Alarm triggered", CFG_phoneNumber);
}
