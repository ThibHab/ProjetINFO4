int sensorValue = 0;  // variable to store the value read
const byte buttonPins[] = {12, 11, 10, 9, 8, 7};

void setup() {
  Serial.begin(9600);  // setup serial
    for (byte i = 0; i < 6; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

/**
    Waits until the user pressed one of the buttons,
    and returns the index of that button
*/
byte readButtons() {
  while (true) {
    for (byte i = 0; i < 6; i++) {
      byte buttonPin = buttonPins[i];
      Serial.println();
      if (digitalRead(buttonPin) == LOW) {
        return i;
      }
    }
    delay(1);
  }
}




void loop() {
// read the input on analog pin 0:
int sensorValue = analogRead(A0);
Serial.println(sensorValue);
delay(1);
sensorValue = analogRead(A1);
Serial.println(sensorValue);
delay(1);
sensorValue = analogRead(A2);
Serial.println(sensorValue);
delay(1);
sensorValue = analogRead(A3);
Serial.println(sensorValue);
sensorValue =  digitalRead((4));
Serial.println(readButtons());

}
