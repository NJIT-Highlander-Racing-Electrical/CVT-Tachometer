#include "Tachometer.h"

int overheatingTempThreshold = 80;

int tempSensor = A0;
int irRecieverEngine = A1;
int irRecieverOutput = A2;

int irLEDPin = 10;

Tachometer engineRPM;
Tachometer outputRPM;

void setup() {
  pinMode(irLEDPin, OUTPUT);
  digitalWrite(irLEDPin, HIGH);

  engineRPM.init(irRecieverEngine, 0.5);
  outputRPM.init(irRecieverOutput, 0.5);
}

void loop() {
  if (calculateTemp > overheatingTempThreshold) {
    // Bad Stuff... (Activate LED on Dashboard)
  }

  // Send RPM Data to CAN-Bus
  engineRPM.getRPM();
  outputRPM.getRPM();
}

double calculateTemp() {
  int tempV = analogRead(tempSensor);
  // Calculation from https://learn.adafruit.com/tmp36-temperature-sensor
  return (((1000 * tempV) - 500) / 10);
}