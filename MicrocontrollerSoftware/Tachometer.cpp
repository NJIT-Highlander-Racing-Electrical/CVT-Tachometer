#include "Tachometer.h"

Tachometer::Tachometer() {  
    this->dataBuffer = 10;

    this->lastUpdateOn = false;
    this->lastTime = 0;
    this->arrayPointer = 0;
}

void Tachometer::init(int pin, int threshold) {
  this->sensorPin = pin;
  this->irRecieverThreshold = threshold;
}

void Tachometer::updateTachometer() {
  if (!this->lastUpdateOn && analogRead(this->sensorPin) > this->irRecieverThreshold) {
    this->lastUpdateOn = true;

    this->timeSinceLastRevolution[this->arrayPointer] = millis() - this->lastTime;
    this->lastTime = millis();

    if (this->arrayPointer < this->dataBuffer - 1) {
      this->arrayPointer++;
    } else {
      this->arrayPointer = 0;
    }
  } else if(analogRead(this->sensorPin) < this->irRecieverThreshold) {
    this->lastUpdateOn = false;
  }
}

double Tachometer::getRPM() {
  int totalTime = 0;
  for (int i = 0; i < this->dataBuffer; i++) {
    totalTime += this->timeSinceLastRevolution[i];
  }

  int timePerRev = totalTime / this->dataBuffer;

  return 1000 / timePerRev;
}

bool Tachometer::getIROn() {
  return this->lastUpdateOn;
}