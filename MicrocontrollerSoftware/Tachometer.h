#ifndef TACHOMETER_H
#define TACHOMETER_H
#include <Arduino.h>

class Tachometer {
  
  private:
    int irRecieverThreshold;
    int dataBuffer;
    int sensorPin;

    bool lastUpdateOn;
    double timeSinceLastRevolution[10];
    double lastTime;
    int arrayPointer;

  public:
    Tachometer();
    void init(int pin, int threshold);
    void updateTachometer();
    double getRPM();
    bool getIROn();
};
#endif