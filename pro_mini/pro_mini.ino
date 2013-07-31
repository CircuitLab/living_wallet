int debugPin = 13;

int delayTime = 1000;

int command;

void setup()
{
  Serial.begin(9600);  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}

void loop()
{
  if (0 < Serial.available()) {
    delayTime = 250;
    
    while (Serial.available()) {
      Serial.println(Serial.read());
    }
//    command = Serial.read();
//    Serial.println(command);
  } else {
    delayTime = 1000;
  }
  
  debugBlink();
}

void debugBlink()
{
  digitalWrite(debugPin, HIGH);
  delay(delayTime);
  digitalWrite(debugPin, LOW);
  delay(delayTime);
}
