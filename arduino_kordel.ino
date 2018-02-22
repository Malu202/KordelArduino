const unsigned char MAX_TOGGLE_TIMES = 32;
const unsigned char MAX_PARALLEL_COMMANDS = 16;
const unsigned char STELLEN_FUER_TOGGLE_TIMES = 5;

class outputCommand {
  private:
    //unsigned short int millisecondsSinceLastToggle = 0;
    unsigned long lastToggleTime;
    unsigned char pinNumber = LED_BUILTIN;
    boolean pinValue;
    unsigned char toggleTimeIndex;
    unsigned short int toggleTimes [MAX_TOGGLE_TIMES];
    unsigned char toggleTimesSize=0;
    
    
  public:
    boolean empty = true;
    boolean initialized = false;
    unsigned char _pinNumber;
    outputCommand() {
    }
    outputCommand(boolean initialValue) {
      empty = false;
      toggleTimeIndex = 0;
      unsigned short int zero = 0;
      for (unsigned char i = 1; i < STELLEN_FUER_TOGGLE_TIMES; i++) {
        toggleTimes[i] = zero;
      }
      lastToggleTime = millis();
      pinValue = initialValue;
      //Damit sofort zu beginn auf die initalValue getoggled wird
      toggleTimes[0] = 0;
      toggleTimesSize = 1;
    }
    void toString() {
      for (unsigned char i = 0; i < MAX_TOGGLE_TIMES; i++) {
        Serial.print(toggleTimes[i]);
        Serial.print(' ');
      }
      Serial.println();
      Serial.print("Pin value: ");
      if (pinValue) Serial.println("true");
      else Serial.println("false");
    }
    void addToggleTime(unsigned short int downloadingToggleTime) {      
          toggleTimes[toggleTimesSize] = downloadingToggleTime;
          toggleTimesSize++;      
    }
    boolean update() {
      if (initialized) {
        //millisecondsSinceLastToggle += elapsedMilliseconds;
        //if (millisecondsSinceLastToggle >= toggleTimes[toggleTimeIndex]) {
        if ((millis() - lastToggleTime) >= toggleTimes[toggleTimeIndex]) {
          Serial.println(millis() - lastToggleTime);
          if (toggleTimeIndex == toggleTimesSize) {
            initialized = false;
            return false;
          }
          digitalWrite(pinNumber, pinValue);
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
}

unsigned short j;
outputCommand downloadingCommand;
char downloadingToggleTime[STELLEN_FUER_TOGGLE_TIMES];
unsigned char i = 0;

void loop() {
  long before = micros();
  for (j = 0; j < MAX_PARALLEL_COMMANDS; j++) {
    if (commands[j].empty) {
      break;
    } else {
      commands[j].update();
    }
  }
  //Serial.println(micros() - before);
  
  if (i == STELLEN_FUER_TOGGLE_TIMES) {      
    downloadingCommand.addToggleTime(downloadingToggleTime[0] * 10000 + downloadingToggleTime[1] * 1000 + downloadingToggleTime [2] * 100 + downloadingToggleTime[3] * 10 + downloadingToggleTime[4]);
    i=0;
  } else if (Serial.available()) {

    char newChar = Serial.read();
    byte newByte = newChar;

    switch (newChar) {
      case 'H' :
        //Serial.println('H');
        downloadingCommand = outputCommand(true);
        break;
      case 'L' :
        //Serial.println('L');
        downloadingCommand = outputCommand(false);
        break;
      case 'E' :
        //Serial.println('E');
        downloadingCommand.initialized = true;
        for (unsigned char k = 0; k < MAX_PARALLEL_COMMANDS; k++) {
          if (commands[k].empty){
            commands[k] = downloadingCommand;
            break;
          }
        }
        downloadingCommand.toString();
        break;
      default :
        //Serial.println("default");
        downloadingToggleTime[i] = newChar - 48;
        i++;          
    }
    //Serial.print("Looptime: ");
  }
}


