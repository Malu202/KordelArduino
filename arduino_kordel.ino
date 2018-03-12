const unsigned char MAX_TOGGLE_TIMES = 32;
const unsigned char MAX_PARALLEL_COMMANDS = 16;
const unsigned char MAX_IR_VALUES = 128;
const unsigned char STELLEN_FUER_IR_TOGGLE_TIMES = 5;
const unsigned char STELLEN_FUER_TOGGLE_TIMES = 5;
const unsigned char STELLEN_FUER_PIN_NUMMERN = 3;
// #include "IRLibAll.h"
#include <IRLibSendBase.h>
#include <IRLib_HashRaw.h> 

class NumberAssembler {
private:
  unsigned char index;
  unsigned char chars[19]; //länge eines unsigned long long int für assemblierungsfaktor
  unsigned char size;
  long long int assemble(){
    unsigned long long int assemblierungsfaktor = 1;
    unsigned long long int output = 0;
    for(unsigned char i = index; i>=0; i--){            
      output += chars[i] * assemblierungsfaktor;
      assemblierungsfaktor *= 10;
    }
    NumberAssembler();
    return output;
  }
public:
  NumberAssembler(){
    index = 0;
  }
  NumberAssembler(unsigned char sizee){
    index = 0;
    size = sizee;
  }
  void add(unsigned char newChar){
    chars[index] = newChar;
    index++;
  }

  unsigned char readUnsignedChar(){
    return assemble();
  }
  boolean isDone(){
    if(index > size){
      return true;
    } else{
      return false;
    }
  }
};

class PinNumberAssembler{
private:
  NumberAssembler numberAssembler = NumberAssembler(STELLEN_FUER_PIN_NUMMERN);
  boolean ignoringState = true;
public:
  unsigned char readPinNumber(){
    ignoringState = true;
    return numberAssembler.readUnsignedChar();
  }
  unsigned char add(unsigned char newChar){
    if(!ignoringState){
      numberAssembler.add(newChar);
    } else {
      if(newChar == 'D'){
        ignoringState = false;
      }
    }
  }
  boolean isDone(){
    return numberAssembler.isDone();
  }

};

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

  class RawIrCommand {
  private:
    const unsigned short int DEFAULT_FREQUENCY = 38.000;
    unsigned short int frequency = DEFAULT_FREQUENCY;    
    IRsendRaw mySender;
    // uint16_t irSignal[] = {8967, 4470, 552, 552, 552, 1683, 552, 1683, 552, 1683, 552, 1683, 552, 1683, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 1683, 552, 39891, 8967, 2235, 552};
    uint16_t irSignal[MAX_IR_VALUES];    
    unsigned char arrayIndex = 0;
    unsigned char byteIndex = 0;
    
    NumberAssembler toggleTimeAssembler;
    void addToggleTime(unsigned char value){
      irSignal[arrayIndex] = value;
      arrayIndex++;
    }
  
  public:
    void add(char value){
      if(byteIndex == STELLEN_FUER_IR_TOGGLE_TIMES - 1){
        toggleTimeAssembler.add(value);
      } else{
        addToggleTime(toggleTimeAssembler.readUnsignedChar());
      }
    }
    void setFrequency(unsigned short int hz){
      frequency = hz;
    }
    void execute(){
      mySender.send(irSignal, arrayIndex+1, 38);
      arrayIndex = 0;
      frequency = DEFAULT_FREQUENCY;
    }
  };

  RawIrCommand rawIrCommand;
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
  
  
  boolean download = true;
  unsigned short parrallelCommandIndex;
  char mode = ' ';
  outputCommand downloadingCommand;
void loop(){
  for (parrallelCommandIndex = 0; parrallelCommandIndex < MAX_PARALLEL_COMMANDS; parrallelCommandIndex++) {
    if (commands[parrallelCommandIndex].empty) {
      break; //not sure
    } else {
      commands[parrallelCommandIndex].update();
    }
  }
  
  
  if(Serial.available()) {
    char newChar = Serial.read();
    
    switch (mode) {
      case 'H':
      case 'L':
        downloadDigitalOutputCommand(newChar);
        break;
      default:
        mode = newChar;   
        //initialization
        switch(newChar){
          case 'E':
          mode == ' ';
          break;
          case 'H':
          downloadingCommand = outputCommand(true);
          break;          
          case 'L':
          downloadingCommand = outputCommand(false);
          break;
        }
      }
    }
  }
//unsigned char pinNumberIndex;
PinNumberAssembler pinNumberAssembler;
NumberAssembler toggleTimeAssembler = NumberAssembler(STELLEN_FUER_IR_TOGGLE_TIMES);
void downloadDigitalOutputCommand(unsigned char newChar){
  if(pinNumberAssembler.isDone()){
    toggleTimeAssembler.add(newChar);
    if(toggleTimeAssembler.isDone()){
      downloadingCommand.addToggleTime(toggleTimeAssembler.readUnsignedChar());
    }
  }
else if(newChar == 'E'){  
  downloadingCommand.initialized = true;
  for (unsigned char k = 0; k < MAX_PARALLEL_COMMANDS; k++) {
    if (commands[k].empty){
      commands[k] = downloadingCommand;
      break;
    }
  }
} else {
  pinNumberAssembler.add(newChar);
  if(pinNumberAssembler.isDone()){
    downloadingCommand.pinNumber = pinNumberAssembler.readPinNumber();
  }
}
}

/*
void sendRawIr(uint8_t frequency){
  uint16_t irSignal[] = {8967, 4470, 552, 552, 552, 1683, 552, 1683, 552, 1683, 552, 1683, 552, 1683, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 552, 552, 1683, 552, 1683, 552, 39891, 8967, 2235, 552};
  unsigned char size = sizeof(irSignal);
  //mySender.send(irSignal,size, 38);
  Serial.println(size);
}
*/
