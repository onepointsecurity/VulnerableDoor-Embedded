#ifndef GSM_h
#define GSM_h

void gsm_setup();
void gsm_sendBlindCommand(const char * command);
void gsm_sendCommand(const char * command, char * buf);
void gsm_sendCommand(const char * command);
void gsm_init();

int gsm_available();
String gsm_readString();

void gsm_sendsms(String text, String number);
#endif
