#ifndef TACHOMETER_H
#define TACHOMETER_H
#include <Arduino.h>

class Tachometer {
  
  private:
    double irRecieverThreshold;
    int dataBuffer;
    int sensorPin;

    bool lastUpdateOn;
    double timeSinceLastRevolution[10];
    double lastTime;
    int arrayPointer;

  public:
    Tachometer();
    void init(int pin, double threshold);
    void updateTachometer();
    double getRPM();
};
#endif