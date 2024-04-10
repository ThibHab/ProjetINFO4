const int analogInPin[4] = {A0, A1, A2, A3};  // Analog input pin that the potentiometer is attached to
const byte buttonPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};   // Digital pins serving as input read up to 9 for I/O and 1 for error detection
const int io = 10;
// Here the first 4 are the VCO output buttons

int sensorValueArray[4] = {0, 0, 0, 0};      // value read from the potentiometer

// Init
void setup() {
  Serial.begin(9600); // start the serial output with a 9600 baud rate
  
  for (byte i = 0; i < io; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  
  Serial.setTimeout(1); // set a timeout before the start
}

// Useful functions
void readAnalog() {//The function that will read and store the value of the potentiometer
  for (int i = 0; i < 4; i++) {
    if (!Serial.available()) { // if there is nothing to read on the serial output
      sensorValueArray[i] = analogRead(analogInPin[i]);
      delay(2); // just here to not have too many values to read
    }
  }
}

void readDigital() {//Read The digitalPins and call the handle connection
  
  Serial.print("/"); //We want the output to be /x:y/ to represent a connection between pin x and y
  for (byte i = 0; i < io + 1; i++) {
    pinMode(buttonPins[i], OUTPUT);
    digitalWrite(buttonPins[i], LOW); // iteration over all pins 
    for (byte j = i + 1; j < io + 1; j++) {
      delay(20);

      if (digitalRead(buttonPins[j]) == LOW) { // we check if there is current passing through
        // Meaning there is a connection between the current pins labeled as output and the one labeled as input
        Serial.print(buttonPins[i]);
        Serial.print(":");
        Serial.print(buttonPins[j]);
        Serial.print("/");
      }
    }
    pinMode(buttonPins[i], INPUT);
    delay(20);
  }
}

void loop() {
  Serial.println();
  readAnalog();
  Serial.println(sensorValueArray[0]);
  delay(200);
  Serial.println(sensorValueArray[1]);
  Serial.println(sensorValueArray[2]);
  delay(2);
  Serial.println(sensorValueArray[3]);
  delay(200);
  readDigital();
}
