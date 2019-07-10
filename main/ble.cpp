#include <SoftwareSerial.h>
#include "Arduino.h"
#include "debug.h"
#include "serial.h"
#include "led.h"
#include "common.h"
#include <rBase64.h>
#include "base64.hpp"
#define BLE_BUF_SIZE 100
#define BLE_CIPHER_KEY_1 "My"
#define BLE_CIPHER_KEY_2 "Sup"
#define BLE_CIPHER_KEY_3 "3r"
#define BLE_CIPHER_KEY_4 "K3y"
#define ENCRYPTION_ENABLED true
#define BLE_RECEIVED_CHAR_LEN 20

String CFG_userAllowedMAC="000000000000";
String CFG_technicalSupportMAC = SUPPORT_MAC;
bool macRestrictionActive = false;
bool deviceConnected = false;

SoftwareSerial ble(7,6);
rBase64generic<BLE_RECEIVED_CHAR_LEN> mybase64;


/*
 * This is highly insecure cipher. Never do that in real life.
 * Some crypto tips: Never use the same key ewice
 * Some general tips: Do not hardcode crypto key materials
 */

void ble_encrypt_decrypt(char * input, char * output, int len){

  // Build Key (this way, it is not visible in strings on the firmware dump)
  String key = String(BLE_CIPHER_KEY_1) + String(BLE_CIPHER_KEY_2) + String(BLE_CIPHER_KEY_3) + String(BLE_CIPHER_KEY_4);
 
  for(int i = 0; i<len; i++){
    output[i] = input[i] ^ key.charAt(i%key.length());
  }
  output[len]='\0';
}

void ble_encrypt(char * plaintext, char * cyphertext){
  if (ENCRYPTION_ENABLED){

    int plaintext_length = strlen(plaintext);
    char encrypted[plaintext_length+1];
    
    // XOR Encryption
    ble_encrypt_decrypt(plaintext, encrypted, plaintext_length);
    
    // Base64 Encoding

    mybase64.encode(encrypted, plaintext_length);
    strcpy(cyphertext, mybase64.result());

  } else {
    debug_println(F("Warning: Encryption not enabled"));
    strcpy(cyphertext, plaintext);
  }
}

void ble_decrypt(char * cyphertext, char * plaintext){
  if (ENCRYPTION_ENABLED){

    // Base64 Decoding

    mybase64.decode(cyphertext, strlen(cyphertext));
    int decLen = rbase64_dec_len(cyphertext, strlen(cyphertext));
    char decoded[decLen+1];
    memcpy(decoded, mybase64.result(), decLen + 1);

    // XOR Decryption
    char decrypted[decLen+1];
    ble_encrypt_decrypt(decoded, decrypted, decLen);

    strcpy(plaintext, decrypted);
    
  } else {
    debug_println(F("Warning: Encryption not enabled"));
    strcpy(plaintext, cyphertext);
  }
}

String ble_decrypt(String cyphertextString){
  char * cyphertext = new char[cyphertextString.length() + 1];
  strcpy(cyphertext,cyphertextString.c_str());

  char plaintext[BLE_BUF_SIZE];
  ble_decrypt(cyphertext, plaintext);

  return String(plaintext);
}

void ble_sendCommand(const char * command, char * buf){
  debug_print(F("[BLE] command sent:  "));
  debug_println(command);
  ble.print(command);
  ble.flush();

  // Wait for an answer from module
  unsigned long timeout = millis()+2000;
  while(millis()<timeout && not ble.available()){}

  if(millis()>timeout){
    // No answer from module
    buf[0] = '\0';
    serial_println(F("[ERROR] No answer from BLE module"));
  } else {
    // Read module answer
    int i = 0;
    timeout = millis()+100; // Wait a little to be sure there are no character left
    while (ble.available() > 0 or millis()<timeout) {
      if(ble.available()){
        buf[i] = ble.read();
        i += 1;
        timeout = millis()+100;
      }
    }
    //end the string
    buf[i] = '\0';    
    
    debug_print(F("[BLE] received reply: "));
    debug_println(buf);
  
  }
}

void ble_sendCommand(const char * command){
  char buf[BLE_BUF_SIZE];
  ble_sendCommand(command, buf);
}

void ble_connect(){
  deviceConnected = true;
}

void ble_disconnect(){
  ble_sendCommand("AT");
  deviceConnected = false;
}

bool ble_connectionAllowed(String device){
  if (not macRestrictionActive) {
    return true;
  } else {
    if (device.equals(CFG_userAllowedMAC)){
      ble_connect();
      return true;
    } else if (device.equals(CFG_technicalSupportMAC)){
      serial_println(F("Warning: Connection with the technical support device."));
      return true;
    } else {
      return false;
    }
  }
}

String ble_getMAC(){
  return CFG_userAllowedMAC;
}


char ble_read(){
  return(ble.read());
}

String ble_readString(){
  return(ble.readString());
}

void ble_print(String value){
  debug_print(F("Sent on BLE: "));
  debug_println(value);
  ble.print(value);
  ble.flush();
}

void ble_println(String value){
  debug_print(F("Sent on BLE: "));
  debug_println(value);
  ble.println(value);
  ble.flush();
}

void ble_send_ciphered(String value){
  char * plaintext = new char[value.length() + 1];
  strcpy(plaintext,value.c_str());

  char cyphertext[value.length()*2]; // This is overestimated but conservative
  ble_encrypt(plaintext, cyphertext);
  ble_println(cyphertext);
}

int ble_available(){
  return ble.available();
}

void ble_restoreFactory(){
  serial_println(F("Restoring BLE Factory Defaults"));
  ble_sendCommand("AT+RENEW");          // Restore Default Config
}

void ble_allowMAC(String device){
  if (not device.equals(CFG_userAllowedMAC) and not device.equals(CFG_technicalSupportMAC)){
    CFG_userAllowedMAC = device;
    serial_print(F("User MAC address recorded: "));
    serial_println(CFG_userAllowedMAC); 
    debug_println(F("Only this device will be able to connect when Config mode will be left."));
  }
}


void ble_setup(){
  serial_print(F("Starting BLE"));

  ble.begin(9600);

  //wait some time
  delay(100);
  ble_sendCommand("AT");                  // Disconnect remote device
  serial_print(".");
  ble_sendCommand("AT+RESET");            // Restart Module  
  serial_print(".");
  delay(1000);
  ble_sendCommand("AT+VERR?");            // Query BLE Module Version
  serial_print(".");
  ble_sendCommand("AT+ADDR?");            // Query BLE Module Address
  serial_print(".");
  ble_sendCommand("AT+NAMEVulnDoor");    // BLE Name SmartDoor
  serial_print(".");
  ble_sendCommand("AT+PASS012345");       // PIN Code set to 012345
  serial_print(".");
  ble_sendCommand("AT+UUID0xFFE0");       // Service UUID
  serial_print(".");
  ble_sendCommand("AT+CHAR0xFFE1");       // Characteristics UUID
  serial_print(".");
  delay(300); // Experience shows that we need to wait a little here...
  ble_sendCommand("AT+NOTI1");            // Notify when link ESTABLISHED or LOST
  serial_print(".");
  ble_sendCommand("AT+NOTP1");            // Add MAC Addr to notification
  serial_print(".");
  ble_sendCommand("AT+TYPE0");            // No PIN Used
  serial_print(".");  
  ble_sendCommand("AT+RESET");            // Restart Module
  serial_print(".");
  delay(1000);
  serial_println("Done");
  ble_println("READY");

}

void ble_setMode(int mode){
  switch(mode){
    case MODE_NORMAL:
      ble_sendCommand("AT");                  // Disconnect any connected device
      ble_sendCommand("AT+RESET");
      delay(300);
      ble_sendCommand("AT+ROLE0");            // Role is Peripheral (Not Central)
      delay(300);
      ble_sendCommand("AT+ADVI0");            // Advertise period: 100ms
      delay(300);
      ble_sendCommand("AT+ADTY0");            // Advertising, ScanResponse, Connectable
      macRestrictionActive = true;
      break;
    case MODE_CONFIG:
      ble_sendCommand("AT");                  // Disconnect any connected device
      ble_sendCommand("AT+ROLE0");            // Role is Peripheral (Not Central)
      ble_sendCommand("AT+ADVI0");            // Advertise period: 100ms
      ble_sendCommand("AT+ADTY0");            // Advertising, ScanResponse, Connectable
      macRestrictionActive = false;
      serial_println(F("Any BLE device can bond"));
      break;
    default:
      debug_println(F("ERROR: Unknown BLE Mode"));
      break;
  }
}

bool ble_isMACConfigured(){
  return not CFG_userAllowedMAC.equals("000000000000");
}

bool ble_isConnected(){
  return deviceConnected;
}

void ble_listen(){
  if (not ble.isListening()){
    debug_println("Listening");
    ble.listen();
  }
}
