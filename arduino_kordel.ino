const unsigned char MAX_TOGGLE_TIMES = 32;
const unsigned char MAX_PARALLEL_COMMANDS = 16;
const unsigned char STELLEN_FUER_TOGGLE_TIMES = 5;
const unsigned char STELLEN_FUER_PIN_NUMMERN = 3;
// #include "IRLibAll.h"
#include <IRLibSendBase.h>
#include <IRLib_HashRaw.h> 
class outputCommand {
  private:
    unsigned long lastToggleTime;
    //unsigned char pinNumber = LED_BUILTIN;
    boolean pinValue;
    unsigned char toggleTimeIndex;
    unsigned short int toggleTimes [MAX_TOGGLE_TIMES];
    unsigned char toggleTimesSize;
    
  public:
    boolean empty = true;
    boolean initialized = false;
    unsigned char pinNumber = LED_BUILTIN;
    outputCommand() {
    }
    outputCommand(boolean initialValue) {
      empty = false;
      toggleTimeIndex = 0;
      unsigned short int zero = 0;
      for (unsigned char toggleTimeStelle = 1; toggleTimeStelle < STELLEN_FUER_TOGGLE_TIMES; toggleTimeStelle++) {
        toggleTimes[toggleTimeStelle] = zero;
      }
      lastToggleTime = millis();
      pinValue = initialValue;
      //Damit sofort zu beginn auf die initalValue getoggled wird
      toggleTimes[0] = 0;
      toggleTimesSize = 1;
    }
    void toString() {
      for (unsigned char toggleTimeStelle = 0; toggleTimeStelle < MAX_TOGGLE_TIMES; toggleTimeStelle++) {
        Serial.print(toggleTimes[toggleTimeStelle]);
        Serial.print(' ');
      }
      Serial.println();
      Serial.print(F("Pin number: "));
      Serial.println(pinNumber);
      Serial.print(F("Pin value: "));
      if (pinValue) Serial.println(F("true"));
      else Serial.println(F("false"));
    }
    void addToggleTime(unsigned short int downloadingToggleTime) {      
          toggleTimes[toggleTimesSize] = downloadingToggleTime;
          toggleTimesSize++;      
    }
    boolean update() {
      if (initialized) {
        if ((millis() - lastToggleTime) >= toggleTimes[toggleTimeIndex]) {
          //Serial.println(millis() - lastToggleTime);
          if (toggleTimeIndex == toggleTimesSize) {
            initialized = false;
            return false;
          }
          if(pinValue) digitalWrite(pinNumber, HIGH);
          else digitalWrite(pinNumber, LOW);
          pinValue = !pinValue;
          lastToggleTime = millis();
          toggleTimeIndex++;
        }
      }
      return true;
    }
  };
  
  outputCommand commands[MAX_PARALLEL_COMMANDS];
  void setup() {
    //Serial.begin(230400); //E wird nicht mehr gesendet
    //Serial.begin(57600);
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    for (unsigned char i = 2; i<=13;i++){
      pinMode(i, OUTPUT);      
    }
    sendRawIr(38);
    Serial.println(F("initialized"));
  }
  
  unsigned short j;
  outputCommand downloadingCommand;
  char downloadingToggleTime[STELLEN_FUER_TOGGLE_TIMES];
  unsigned char toggleTimeStelle = 0;
  
  char pinString[STELLEN_FUER_PIN_NUMMERN];
  unsigned char pinStringStelle = 0;
  
  
  void loop() {
    for (j = 0; j < MAX_PARALLEL_COMMANDS; j++) {
      if (commands[j].empty) {
        break;
      } else {
        commands[j].update();
      }
    }
    
    if (toggleTimeStelle == STELLEN_FUER_TOGGLE_TIMES) {      
      downloadingCommand.addToggleTime(downloadingToggleTime[0] * 10000 + downloadingToggleTime[1] * 1000 + downloadingToggleTime [2] * 100 + downloadingToggleTime[3] * 10 + downloadingToggleTime[4]);
      toggleTimeStelle=0;
    } else if (pinStringStelle == STELLEN_FUER_PIN_NUMMERN){
    //long before = micros();
    for (unsigned char i = 0; i < STELLEN_FUER_PIN_NUMMERN; i++){
      if((pinString[i] + 48) == 'D'){
        unsigned char faktor = 1;
        downloadingCommand.pinNumber = 0;
        for (unsigned char j = STELLEN_FUER_PIN_NUMMERN-1; j > i; j--){
          downloadingCommand.pinNumber += pinString[j]*faktor;
          faktor *= 10;
        }
        pinStringStelle++;
        break;
      }
    } 
    //Serial.println(micros() - before);
  } else if(Serial.available()) {
    char newChar = Serial.read();
    switch (newChar) {
      case 'H' :
      downloadingCommand = outputCommand(true);
        break;
        case 'L' :
        downloadingCommand = outputCommand(false);
        break;
        case 'E' :
        downloadingCommand.initialized = true;
        for (unsigned char k = 0; k < MAX_PARALLEL_COMMANDS; k++) {
          if (commands[k].empty){
            commands[k] = downloadingCommand;
            break;
          }
        }
        pinStringStelle = 0;
        //downloadingCommand.toString();
        break;
      default :
        if(pinStringStelle < STELLEN_FUER_PIN_NUMMERN){
          pinString[pinStringStelle] = newChar - 48;
          pinStringStelle++;
        }
        else {
          downloadingToggleTime[toggleTimeStelle] = newChar - 48;
          toggleTimeStelle++;          
        }
    }
  }
}



IRsendRaw mySender;
void sendRawIr(uint8_t frequency){
  uint16_t irSignal[] = {8967, 4470, 552, 552, 552, 1683, 552, 1683, 552, 1683, 552, 1683, 552, 1683, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 1683, 552, 39891, 8967, 2235, 552};
  unsigned char size = sizeof(irSignal);
  //mySender.send(irSignal,size, 38);
  Serial.println(size);
}