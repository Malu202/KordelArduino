const unsigned char MAX_TOGGLE_TIMES = 32;
const unsigned char MAX_PARALLEL_COMMANDS = 16;
const unsigned char STELLEN_FUER_TOGGLE_TIMES = 5;

class outputCommand {
  private:
    //unsigned short int millisecondsSinceLastToggle = 0;
    unsigned short int lastToggleTime;
    unsigned char pinNumber;
    boolean pinValue;
    unsigned char toggleTimeIndex;
    unsigned short int toggleTimes [MAX_TOGGLE_TIMES];

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
      for (unsigned char i = 1; i < MAX_TOGGLE_TIMES; i++) {
        toggleTimes[i] = zero;
      }
      lastToggleTime = millis();
      pinValue = initialValue;
      //Damit sofort zu beginn auf die initalValue getoggled wird
      toggleTimes[0] = 0;
    }
    void toString() {
      for (unsigned char i = 0; i <= MAX_TOGGLE_TIMES; i++) {
        Serial.print(toggleTimes[i]);
        Serial.print(' ');
      }
      Serial.println();
      Serial.print("Pin value: ");
      if (pinValue) Serial.println("true");
      else Serial.println("false");
    }
    void addToggleTime(unsigned short int toggleTime) {
      for (unsigned char i = 1; i <= MAX_TOGGLE_TIMES; i++) {
        if (toggleTimes[i] == 0) {
          toggleTimes[i] = toggleTime;
          break;
        }
      }
    }
    boolean update() {
      if (initialized) {
        //millisecondsSinceLastToggle += elapsedMilliseconds;
        //if (millisecondsSinceLastToggle >= toggleTimes[toggleTimeIndex]) {

        if ((millis() - lastToggleTime) >= toggleTimes[toggleTimeIndex]) {
          if (toggleTimes[toggleTimeIndex] == 0) {
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
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println('hi');
}

unsigned short j;
outputCommand downloadingCommand;
void loop() {
  for (j = 0; j < MAX_PARALLEL_COMMANDS; j++) {
    if (commands[j].empty) {
      break;
    } else {
      commands[j].update();
    }
  }
  if (Serial.available()) {
    long before = millis();

    char newChar = Serial.read();
    byte newByte = newChar;

    switch (newChar) {
      case 'H' :
        Serial.println('H');
        downloadingCommand = outputCommand(true);
        break;
      case 'L' :
        Serial.println('L');
        downloadingCommand = outputCommand(false);
        break;
      case 'E' :
        Serial.println('E');
        for (unsigned char k = 0; k < MAX_PARALLEL_COMMANDS; k++) {
          if (commands[k].empty) commands[k] = downloadingCommand;
          break;
        }
        //downloadingCommand.toString();
        break;
      default :
        Serial.println("default");
        char toggleTime[STELLEN_FUER_TOGGLE_TIMES];
        toggleTime[0] = newChar - 48;
        unsigned char i = 1;
        while (true) {
          if (Serial.available()) {
            toggleTime[i] = Serial.read() - 48;
            i++;
            if (i >= STELLEN_FUER_TOGGLE_TIMES) break;
          }
        }
        unsigned short int toggleTimeAssembly = toggleTime[0] * 10000 + toggleTime[1] * 1000 + toggleTime [2] * 100 + toggleTime[3] * 10 + toggleTime[4];
        downloadingCommand.addToggleTime(toggleTimeAssembly);
    }
    Serial.print("Looptime: ");
    Serial.println(millis() - before);

  }
}


