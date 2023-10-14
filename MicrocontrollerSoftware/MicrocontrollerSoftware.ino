class Tachometer{
  double irRecieverThreshold = 0.5;
  int dataBuffer;
  int sensorPin;

  bool lastUpdateOn = false;
  double timeSinceLastRevolution[];
  double lastTime = 0;
  int arrayPointer = 0;

  Tachometer::Tachometer(int pin, int db, double threshold) {
    sensorPin = pin;
    dataBuffer = db;
    irRecieverThreshold = threshold;
    
    timeSinceLastRevolution = new double[dataBuffer];
  }

  void updateTachometer() {
    if (!lastUpdateOn[0] && analogRead(sensorPin) > irRecieverEngineThreshold) {
      lastUpdateOn[0] = true;

      timeSinceLastRevolution[arrayPointer] = millis() - lastTime;
      lastTime = millis();

      if (arrayPointer < dataBuffer - 1) {
        arrayPointer++;
      } else {
        arrayPointer = 0;
      }
    }
  }

  double getRPM() {
    totalTime = 0;
    for (int i = 0; i < dataBuffer; i++) {
      totalTime += timeSinceLastRevolution[i];
    }

    timePerRev = totalTime / dataBuffer;

    return 1000 / timePerRev;
  }
}



int overheatingTempThreshold = 80;

int tempSensor = A0;
int irRecieverEngine = A1;
int irRecieverOutput = A2;

int irLEDPin = 10;

Tachometer engineRPM;
Tachometer outputRPM;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  engineRPM = new Tachometer(irRecieverEngine, 10, 0.5);
  outputRPM = new Tachometer(irRecieverOutput, 10, 0.5);
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
  tempV = analogRead(tempSensor);
  // Calculation from https://learn.adafruit.com/tmp36-temperature-sensor
  return (((1000 * tempV) - 500) / 10);
}