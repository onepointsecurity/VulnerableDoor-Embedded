#ifndef Debug_h
#define Debug_h

void debug_print(const char* text);
void debug_println(const char* text);

void debug_print(const __FlashStringHelper* text);
void debug_println(const __FlashStringHelper* text);

void debug_println(const String& text); 
#endif
