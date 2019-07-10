#ifndef BLE_h
#define BLE_h

void ble_setup();
void ble_sendCommand(const char * command, char * buf);
void ble_sendCommand(const char * command);
char ble_read();
String ble_readString();
void ble_print(String value);
void ble_println(String value);
int ble_available();
void ble_allowMAC(String device);
bool ble_connectionAllowed(String device);
void ble_disconnect();
bool ble_isMACConfigured();
void ble_restoreFactory();
void ble_setMode(int);
String ble_getMAC();
bool ble_isConnected();
void ble_listen();

void ble_encrypt(char * plaintext, char * cyphertext);
void ble_decrypt(char * cyphertext, char * plaintext);
String ble_decrypt(String cyphertext);
void ble_send_ciphered(String value);

#endif
