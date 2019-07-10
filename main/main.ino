#include "ble.h"
#include "door.h"
#include "led.h"
#include "common.h"
#include "debug.h"
#include "gsm.h"
#include "serial.h"

#define UNKNOWN 0
#define READ 1
#define WRITE 2
#define PHONE_NUMBER 3
#define SERIAL 4
#define BLE 5
#define GSMX 6
#define MAC_ADDRESS 7

#define BLE_BUF_SIZE 100
#define GSM_BUF_SIZE 100

char serial_char;
String serial_rx = "";
String ble_rx = "";
String gsm_rx = "";

void setup()
{

  // Hardware
  LED_init();
  serial_init();
  door_init();

  setMode(MODE_INIT);

  serial_println(F("Booting..."));

  // GSM
  gsm_setup(),

  // BLE
  ble_setup();

  serial_println(F("Device Ready"));

  // Start in Config Mode
  setMode(MODE_CONFIG);

  serial_println(F("+------------------------------------------------+"));
  serial_println(F("| To list all available commands, enter \"help\"   |"));
  serial_println(F("+------------------------------------------------+"));
  
  loop();

}

void sendPhoneNumber(){
  String phoneNumber = getPhoneNumber();
  serial_println(phoneNumber); 
  ble_send_ciphered(phoneNumber);
}

void sendMAC(){
  String configuredMacAddress = ble_getMAC();
  debug_println(configuredMacAddress); 
  ble_send_ciphered(configuredMacAddress);
}

void handleCommand(int origin, const String& command){

  if(origin == SERIAL){
    // Commands from Debug UART
    serial_print(F("$(Serial): "));
    serial_println(command); 
    if(command.equals("mode normal")){
      setMode(MODE_NORMAL);
      return;
    } else if (command.equals("mode config")){
      setMode(MODE_CONFIG);
      return;
    } else if (command.equals("resetble")){
      ble_setup();
      ble_setMode(getMode());
      return;   
    } else if (command.equals("resetgsm")){
      gsm_init();
      return;   
    } else if (command.equals("eraseble")){
      ble_restoreFactory();
      ble_setup();
      return;          
    } else if (command.equals("open")){
      door_open();
      return;          
    } else if (command.equals("close")){
      door_close();
      return;   
    } else if (command.startsWith("ble:AT")){
      serial_println(F("RAW Bluetooth command"));
      char ble_output[BLE_BUF_SIZE];
      ble_sendCommand((char *)command.substring(4,command.length()).c_str(), ble_output);
      serial_println(ble_output);
      return;    
    } else if (command.startsWith("gsm:AT")){
      serial_println(F("RAW GSM command"));
      char gsm_output[GSM_BUF_SIZE];
      gsm_sendCommand((char *)command.substring(4,command.length()).c_str(), gsm_output);
      serial_println(gsm_output);
      return;    
    } else if (command.equals("showmac")){
      serial_print(F("Recorded Bluetooth MAC: "));
      serial_println(ble_getMAC());
      return; 
    } else if (command.equals("r:tel")){
      String phoneNumber = getPhoneNumber();
      serial_println(phoneNumber); 
      return;
    } else if (command.equals("r:mac")){
      String configuredMacAddress = ble_getMAC();
      serial_println(configuredMacAddress); 
      return;
    } else if (command.startsWith("w:tel")) {
      String phone_number = command.substring(5,15);
      savePhoneNumber(phone_number);
      return;
    } else if (command.equals("help")){
      serial_println("");
      serial_println(F("+--------------------------------------+"));
      serial_println(F("|          Available Commands          |"));
      serial_println(F("+----------+---------------------------+"));
      serial_println(F("| mode XXX | Set module to selected    |"));
      serial_println(F("|          | mode: config/normal       |"));
      serial_println(F("| resetble | Reset Bluetooth module    |"));
      serial_println(F("| resetgsm | Reset GSM module          |"));
      serial_println(F("| eraseble | Set Bluetooth factory cfg |"));
      serial_println(F("| showmac  | Show configured MAC       |"));
      serial_println(F("| open     | Simulate opening door     |"));
      serial_println(F("| close    | Simulate closing door     |"));
      serial_println(F("| r:tel    | Read phone number         |"));
      serial_println(F("| w:tel XX | Write phone number XX     |"));
      serial_println(F("| r:mac    | Read saved MAC            |"));
      serial_println(F("| ble:ATXX | BLE chipset AT Commands   |"));
      serial_println(F("| gsm:ATXX | GSM chipset AT Commands   |"));
      serial_println(F("+----------+---------------------------+"));
      return;          
    } else {
      serial_println(F("Unknown command"));
      return;
    }
  } else if(origin == BLE){
    // Commands received from BLE UART
    // BLE Service Commands
    debug_print(F("$(BLE): "));
    debug_println(command); 
    if(command.equals("OK+LOST")){
      debug_println(F("Device disconnected"));
      ble_disconnect();
      return;
    } else if (command.startsWith("OK+CONN")){
      String device;
      device = command.substring(8,20);
      debug_print(F("Device connected: "));
      debug_println((char*)device.c_str());

      if (ble_connectionAllowed(device)){
        debug_println(F("Connection allowed"));
        ble_allowMAC(device);
        delay(1000); 
        ble_println(F("READY"));
      } else {
        serial_print(F("Connection denied from "));
        serial_println((char*)device.c_str());
        ble_disconnect();
      }

      return;      
    }

    // Decipher user command
    String decrypted_command = ble_decrypt(command);
    debug_print(F("$(BLE Decrypted): "));
    debug_println(decrypted_command); 
    
    // User Commands
    // Command READ/WRITE
    int read_write = UNKNOWN;
    if (decrypted_command[0]=='w'){
      read_write = WRITE;
    } else if (decrypted_command[0]=='r'){
      read_write = READ;
    } else {
      serial_print(F("Unknown Command: "));
      serial_println(decrypted_command);
      return;
    }
  
    // Command Parameter Name
    int parameter = UNKNOWN;
    if (decrypted_command[2]=='t' && decrypted_command[3]=='e' && decrypted_command[4]=='l'){
      parameter = PHONE_NUMBER;
    } else if (decrypted_command[2]=='m' && decrypted_command[3]=='a' && decrypted_command[4]=='c'){
      parameter = MAC_ADDRESS;
    } else {
      serial_println(F("Unknown Parameter"));
      return;
    }
  
    // Command Parameter Value
    if (read_write == WRITE && parameter == PHONE_NUMBER){
      String phone_number = decrypted_command.substring(5,15);
      savePhoneNumber(phone_number);
    } else if (read_write == READ && parameter == PHONE_NUMBER){
      sendPhoneNumber();
    } else if (read_write == READ && parameter == MAC_ADDRESS){
      sendMAC();
    }
  } else if(origin == GSMX){
    // Commands received from GSM UART
    // GSM Device Commands
    debug_print(F("$(GSM): "));
    debug_println((char*)command.c_str());
    return;
  }
}

void loop() {

  serial_rx = "";
  ble_rx = "";
  gsm_rx = "";

  // Main Loop
  while(true) {

    door_check();

    // Handle Debug UART input
    while (Serial.available() > 0) {
      serial_char = Serial.read();
      if (serial_char == '\n'){
        handleCommand(SERIAL, serial_rx);
        serial_rx = "";
      } else {
        serial_rx += serial_char;         
      }
    }

    // Handle BLE UART input
    if (ble_available() > 0) {
      ble_rx += ble_readString();
    }     
    if (ble_rx != ""){
      handleCommand(BLE, ble_rx);
      ble_rx = "";
    }

    // Handle GSM UART input
    if (gsm_available() > 0) {
      gsm_rx += gsm_readString();
    }     
    if (gsm_rx != ""){
      handleCommand(GSMX, gsm_rx);
      gsm_rx = "";
    }

    // Check that we are ready to receive BLE input
    ble_listen();
  }  
}
