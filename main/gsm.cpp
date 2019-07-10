#include <SoftwareSerial.h>
#include "Arduino.h"
#include "debug.h"
#include "serial.h"
#include "common.h"
#define GSM_BUF_SIZE 50

SoftwareSerial gsm(9,10);

void gsm_sendBlindCommand(const char * command){
  debug_print(F("[GSM] command sent: "));
  debug_println(command);
  gsm.print(command);
  gsm.print('\r');  // Carriage Return
  gsm.flush();
}

void gsm_sendCommand(const char * command, char * buf){
  gsm.listen();
  debug_print(F("[GSM] command sent: "));
  debug_println(command);
  gsm.print(command);
  gsm.print('\r');  // Carriage Return
  gsm.flush();

  // Wait for an answer from module
  unsigned long timeout = millis()+2000;
  while(millis()<timeout && not gsm.available()){}

  if (gsm.overflow()) {
    serial_println(F("[ERROR] GSM SoftwareSerial overflow!"));
  }
      
  if(millis()>timeout){
    // No answer from module
    buf[0] = '\0';
    serial_println(F("[ERROR] No answer from GSM module"));
  } else {
    // Read module answer
    int i = 0;
    timeout = millis()+300; // Wait a little to be sure there are no character left
    while (gsm.available() > 0 or millis()<timeout) {
      if(gsm.available()){
        // Protect against buffer overflow
        if (i<GSM_BUF_SIZE-1){
          buf[i] = gsm.read();
          i += 1;
        } else {
          gsm.read();
        }
        timeout = millis()+100;
      }
    }
    //end the string
    buf[i] = '\0';    

    debug_print(F("[GSM] received reply: "));
    debug_println(buf);  
  }

}

void gsm_sendCommand(const char * command){
  char buf[GSM_BUF_SIZE];
  gsm_sendCommand(command, buf);
}

int gsm_available(){
  return gsm.available();
}

String gsm_readString(){
  return(gsm.readString());
}

void gsm_sendsms(String text, String number){
  gsm_sendCommand("AT+CMGF=1");
  delay(100);
  String numberCommand = "AT+CMGS=\"" + number + "\"";
  gsm_sendCommand((char *)numberCommand.c_str());
  delay(100);
  text.concat((char)26);
  gsm_sendBlindCommand((char *)text.c_str());
}

void gsm_init(){
  gsm_sendCommand("AT");
  serial_print(".");
  gsm_sendCommand("ATE0");          // Echo OFF
  serial_print(".");
  gsm_sendCommand("ATI");           // Firmware Revision
  serial_print(".");
  gsm_sendCommand("AT+IPR=9600");   // Baudrate 9600bauds
  serial_print(".");
  delay(2000); 
  gsm_sendCommand("AT+CCID");       // Check SIM present // TODO react if not
  serial_print(".");
  gsm_sendCommand("AT+CPIN=0000");  // Enter SIM PIN // TODO react if not  
  serial_print(".");
  delay(4000);                      // Wait 2s for network to register
  serial_print(".");
  gsm_sendCommand("AT+COPS?");      // Return network connected to  
  serial_print(".");
  delay(4000);
  //TODO Wait for connection to network
}

void gsm_setup(){
  
  serial_print(F("Starting GSM"));

  gsm.begin(9600);

  gsm_init();

  serial_println("Done");

}
