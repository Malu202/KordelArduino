char byteRead;
byte commandsRead[1024];
int ledNumber = LED_BUILTIN;
int value;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (Serial.available()) {
    byteRead = Serial.read();
    if (byteRead == 'E') {
      //Serial.println("End of Command");
    }
    Serial.println(byteRead);
    Serial.write(byteRead);
    delay(3000);
    if (value == 1) {
      //digitalWrite(ledNumber, HIGH);
    }
    else if (value == 0) {
      //digitalWrite(ledNumber, LOW);
    }
    else {
      //Serial.println("Unknown Command");
    }
  }
}

