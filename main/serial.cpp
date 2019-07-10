#include "Arduino.h"

void serial_init(){
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Serial port available");
} 

void serial_print(const char* text){
  Serial.print(text);
  Serial.flush();
}

void serial_println(const char* text){
  Serial.println(text);
  Serial.flush();
}

void serial_print(const String text){
  Serial.print(text);
  Serial.flush();
}

void serial_println(const String text){
  Serial.println(text);
  Serial.flush();
}

void serial_print(const __FlashStringHelper* text){
  Serial.print(text);
  Serial.flush();
}

void serial_println(const __FlashStringHelper* text){
  Serial.println(text);
  Serial.flush();
}
