#ifndef COMMON_h
#define COMMON_h

#define MODE_INIT 0
#define MODE_CONFIG 1
#define MODE_NORMAL 2

#define SUPPORT_MAC "DEADBEEF0000"

void setMode(int mode);
int getMode();
bool configValid();
void savePhoneNumber(const String& phoneNumber);
String getPhoneNumber();
void triggerAlarm();
#endif
