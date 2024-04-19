#include "Tachometer.h"

int overheatingTempThreshold = 80;

int tempSensor = A5;
int irRecieverEngine = A0;
int irRecieverOutput = A7;

int irLEDPin = 10;

Tachometer engineRPM;
Tachometer outputRPM;

void setup() {
  Serial.begin(115200);
  Serial.println("test");

  //pinMode(irLEDPin, OUTPUT);
  //digitalWrite(irLEDPin, HIGH);
  
  pinMode(irRecieverEngine, INPUT);

  engineRPM.init(irRecieverEngine, 3400);
  outputRPM.init(irRecieverOutput, 3400);
}

void loop() {
  if (int(calculateTemp) > overheatingTempThreshold) {
    // Bad Stuff... (Activate LED on Dashboard)
  }

  engineRPM.updateTachometer();
  outputRPM.updateTachometer();

  //int sensorValue = analogRead(irRecieverEngine);
  Serial.println(engineRPM.getIROn());
  Serial.println(engineRPM.getRPM());
  delay(100);

  // Send RPM Data to CAN-Bus
  //engineRPM.getRPM();
  //outputRPM.getRPM();
}

double calculateTemp() {
  int tempV = analogRead(tempSensor);
  // Calculation from https://learn.adafruit.com/tmp36-temperature-sensor
  return (((1000 * tempV) - 500) / 10);
}