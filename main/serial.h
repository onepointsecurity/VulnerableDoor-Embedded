#ifndef Serial_h
#define Serial_h

void serial_init();

void serial_print(const char* text);
void serial_print(const String text);
void serial_print(const __FlashStringHelper* text);

void serial_println(const char* text);
void serial_println(const String text);
void serial_println(const __FlashStringHelper* text);
#endif
