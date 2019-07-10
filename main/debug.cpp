#include "Arduino.h"
bool debug_on = false;

void debug_print(const char* text){
  if(debug_on){
    Serial.print(text);
    Serial.flush();
  }
}

void debug_println(const char* text){
  if(debug_on){
    Serial.println(text);
    Serial.flush();
  }
}

void debug_print(const __FlashStringHelper* text){
  if(debug_on){
    Serial.print(text);
    Serial.flush();
  }
}

void debug_println(const __FlashStringHelper* text){
  if(debug_on){
    Serial.println(text);
    Serial.flush();
  }
}

void debug_println(const String& text){
  debug_println((char*)text.c_str()); 
}
