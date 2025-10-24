/*
*
* This program uses two infrared emitter/receiver pairs with comparators to detect revolutions on the primary and
* secondary pulleys of the CVT. The analog reading from the infrared receiver configuration (which acts as a variable
* resistor, so it is wired as a voltage divider) is passed through a comparator with a known reference. Then, this comparator
* output is set up as an interrupt input to the ESP32, and each CVT reading will trigger the ISR. Calculations are done to
* determine the RPM based on the elapsed time between readings, and this data is sent over CAN to the rest of the vehicle's subsystems.
*
* There is also a TMP36 temperature sensor on each module that reports ambient temperature inside of the CVT case.
* Additionally, a GY-906 MLX90614 infrared temperature sensor reads the CVT belt temperature over I2C.
*
*/

#include "BajaCAN.h"
#include <Wire.h>

// Set DEBUG to false for final program; Serial is just used for troubleshooting
#define DEBUG true

#define DEBUG_SERIAL \
  if (DEBUG) Serial

#define PRIMARY_IR 32    // Primary Comparator Output
#define PRIMARY_TEMP 33  // Primary TMP36 Sensor pin

#define SECONDARY_IR 27    // Secondary Comparator Output
#define SECONDARY_TEMP 14  // Secondary TMP36 Sensor pin

// MLX90614 I2C Configuration
#define MLX90614_I2C_ADDR 0x5A
#define MLX90614_OBJECT_TEMP 0x07  // Object temperature register
#define I2C_SDA 21  // ESP32 default SDA pin
#define I2C_SCL 22  // ESP32 default SCL pin

// USER-DEFINED PARAMETERS
const int debugPrintInterval = 100;   // Rate at which we print to Serial monitor (increased from 20 to reduce interference)
const int tempUpdateFrequency = 500;  // Get a new temperature reading every 500 milliseconds
const int numPrimaryTargets = 8;      // Number of independent sensing targets on CVT primary
const int numSecondaryTargets = 3;    // Number of independent sensing targets on CVT secondary

const int timeoutThreshold = 1000000;  // If there are no readings in timeoutThreshold microseconds, reset RPM to zero

// Debounce time in microseconds - minimum time between valid pulses
// 10000us = 10ms corresponds to ~3000 RPM max with 2 targets
// Increase this if you still see oscillation at idle
const unsigned long debounceTimeMicros = 10000;

// END

// Serial Print timing variables (milliseconds)
int lastPrintTime = 0;

// Primary timing variables (microseconds)
volatile unsigned long currentPrimaryReadTime = 0;
volatile unsigned long lastPrimaryReadTime = 0;

// Secondary timing variables (microseconds)
volatile unsigned long currentSecondaryReadTime = 0;
volatile unsigned long lastSecondaryReadTime = 0;

// Temperature timing variables (milliseconds)
unsigned long lastPrimTempReadTime = 0;
unsigned long lastSecTempReadTime = 0;
unsigned long lastBeltTempReadTime = 0;

// Update flags (set in ISR)
volatile bool primaryUpdateFlag = false;
volatile bool secondaryUpdateFlag = false;

// Timeout flags; these get set if the readings timed out so we ignore the first calculation back
bool primaryTimedOut = false;
bool secondaryTimedOut = false;

// Moving average filter for RPM smoothing
const int RPM_FILTER_SIZE = 5;
int primaryRPMBuffer[RPM_FILTER_SIZE] = {0};
int secondaryRPMBuffer[RPM_FILTER_SIZE] = {0};
int primaryBufferIndex = 0;
int secondaryBufferIndex = 0;


// Function Prototypes
void primaryISR();
void readPrimaryTemp();
void checkPrimaryTimeout();

void readSecondaryTemp();
void secondaryISR();
void checkSecondaryTimeout();

void readBeltTemp();

void printData();

void setup() {
  Serial.begin(460800);

  // Initialize I2C for MLX90614
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize CAN using BajaCAN. We want to send every 25 milliseconds (40Hz send rate)
  setupCAN(CVT, 25);

  // Configure pins
  pinMode(PRIMARY_IR, INPUT);
  pinMode(PRIMARY_TEMP, INPUT);
  pinMode(SECONDARY_IR, INPUT);
  pinMode(SECONDARY_TEMP, INPUT);

  // Initialize timing variables to prevent garbage first calculation
  currentPrimaryReadTime = micros();
  currentSecondaryReadTime = micros();

  // Attach interrupts on rising edge
  attachInterrupt(digitalPinToInterrupt(PRIMARY_IR), primaryISR, RISING);
  attachInterrupt(digitalPinToInterrupt(SECONDARY_IR), secondaryISR, RISING);

  DEBUG_SERIAL.println("CVT Tachometer Initialized");
}

void loop() {

  // Check to see if either CVT reading timed out
  checkPrimaryTimeout();
  checkSecondaryTimeout();

  // If primaryUpdateFlag was set in the ISR, we have a new revolution and must calculate RPM
  if (primaryUpdateFlag) {

    primaryUpdateFlag = false;

    if (!primaryTimedOut) {  // Only do this RPM calculation if we have a valid previous reading
      unsigned long elapsed = currentPrimaryReadTime - lastPrimaryReadTime;
      if (elapsed > 0) {  // This will prevent divide-by-zero
        int calculatedRPM = (60000000.0 / elapsed) / numPrimaryTargets;

        if (calculatedRPM < 6000) { // accounts for possibility of accidental back-to-back readings on that would never occur (like 120,000 RPM)
          // Add to moving average buffer
          primaryRPMBuffer[primaryBufferIndex] = calculatedRPM;
          primaryBufferIndex = (primaryBufferIndex + 1) % RPM_FILTER_SIZE;
          
          // Calculate median of buffer to reject outliers
          int sortedBuffer[RPM_FILTER_SIZE];
          memcpy(sortedBuffer, primaryRPMBuffer, sizeof(primaryRPMBuffer));
          
          // Simple bubble sort
          for (int i = 0; i < RPM_FILTER_SIZE - 1; i++) {
            for (int j = 0; j < RPM_FILTER_SIZE - i - 1; j++) {
              if (sortedBuffer[j] > sortedBuffer[j + 1]) {
                int temp = sortedBuffer[j];
                sortedBuffer[j] = sortedBuffer[j + 1];
                sortedBuffer[j + 1] = temp;
              }
            }
          }
          
          // Use median value (middle of sorted array)
          primaryRPM = sortedBuffer[RPM_FILTER_SIZE / 2];
        }
      }
    }

    primaryTimedOut = false;
  }

  // If secondaryUpdateFlag was set in the ISR, we have a new revolution and must calculate RPM
  if (secondaryUpdateFlag) {

    secondaryUpdateFlag = false;

    if (!secondaryTimedOut) {  // Only do this RPM calculation if we have a valid previous reading
      // Calculate RPM based on time between pulses
      unsigned long elapsed = currentSecondaryReadTime - lastSecondaryReadTime;
      if (elapsed > 0) {  // This will prevent divide-by-zero
        int calculatedRPM = (60000000.0 / elapsed) / numSecondaryTargets;

        if (calculatedRPM < 6000) { // accounts for possibility of accidental back-to-back readings on that would never occur (like 120,000 RPM)
          // Add to moving average buffer
          secondaryRPMBuffer[secondaryBufferIndex] = calculatedRPM;
          secondaryBufferIndex = (secondaryBufferIndex + 1) % RPM_FILTER_SIZE;
          
          // Calculate median of buffer to reject outliers
          int sortedBuffer[RPM_FILTER_SIZE];
          memcpy(sortedBuffer, secondaryRPMBuffer, sizeof(secondaryRPMBuffer));
          
          // Simple bubble sort
          for (int i = 0; i < RPM_FILTER_SIZE - 1; i++) {
            for (int j = 0; j < RPM_FILTER_SIZE - i - 1; j++) {
              if (sortedBuffer[j] > sortedBuffer[j + 1]) {
                int temp = sortedBuffer[j];
                sortedBuffer[j] = sortedBuffer[j + 1];
                sortedBuffer[j + 1] = temp;
              }
            }
          }
          
          // Use median value (middle of sorted array)
          secondaryRPM = sortedBuffer[RPM_FILTER_SIZE / 2];
        }
      }
    }

    secondaryTimedOut = false;
  }

  // Read temperatures
  readPrimaryTemp();
  readSecondaryTemp();
  readBeltTemp();

  // Print debug data
  if (DEBUG && (millis() - lastPrintTime) > debugPrintInterval) {
    lastPrintTime = millis();
    printData();
  }
}


/*
*
*   FUNCTION DEFINITIONS
*
*/

void primaryISR() {
  unsigned long now = micros();
  
  // Debounce: ignore pulses that occur too quickly (noise/bounce rejection)
  if (now - currentPrimaryReadTime < debounceTimeMicros) {
    return;
  }
  
  lastPrimaryReadTime = currentPrimaryReadTime;
  currentPrimaryReadTime = now;
  primaryUpdateFlag = true;
}

void readPrimaryTemp() {
  if ((millis() - lastPrimTempReadTime) > tempUpdateFrequency) {
    long total = 0;

    // Average 10 readings
    for (int i = 0; i < 10; i++) {
      total += analogRead(PRIMARY_TEMP);
    }

    // Calculate the average reading
    int primTempReading = total / 10;

    float primTempVoltage = (float)primTempReading * 3.3;
    primTempVoltage /= 4095.0;
    float primTempC = (primTempVoltage - 0.5) * 100;      // converting from 10 mv per degree with 500 mV offset
    primaryTemperature = (primTempC * 9.0 / 5.0) + 32.0;  // determining temperature in F from C

    lastPrimTempReadTime = millis();
  }
}

void checkPrimaryTimeout() {
  // Check for timeout and reset RPM to zero if no pulses
  if ((micros() - lastPrimaryReadTime) > timeoutThreshold) {
    primaryRPM = 0;
    primaryTimedOut = true;
  }
}

void secondaryISR() {
  unsigned long now = micros();
  
  // Debounce: ignore pulses that occur too quickly (noise/bounce rejection)
  if (now - currentSecondaryReadTime < debounceTimeMicros) {
    return;
  }
  
  lastSecondaryReadTime = currentSecondaryReadTime;
  currentSecondaryReadTime = now;
  secondaryUpdateFlag = true;
}

void readSecondaryTemp() {
  if ((millis() - lastSecTempReadTime) > tempUpdateFrequency) {
    long total = 0;

    // Average 10 readings
    for (int i = 0; i < 10; i++) {
      total += analogRead(SECONDARY_TEMP);
    }

    // Calculate the average reading
    int secTempReading = total / 10;

    float secTempVoltage = (float)secTempReading * 3.3;
    secTempVoltage /= 4095.0;
    float secTempC = (secTempVoltage - 0.5) * 100;         // converting from 10 mv per degree with 500 mV offset
    secondaryTemperature = (secTempC * 9.0 / 5.0) + 32.0;  // determining temperature in F from C

    lastSecTempReadTime = millis();
  }
}

void checkSecondaryTimeout() {
  // Check for timeout and reset RPM to zero if no pulses
  if ((micros() - lastSecondaryReadTime) > timeoutThreshold) {
    secondaryRPM = 0;
    secondaryTimedOut = true;
  }
}

void readBeltTemp() {
  if ((millis() - lastBeltTempReadTime) > tempUpdateFrequency) {
    // Request object temperature from MLX90614
    Wire.beginTransmission(MLX90614_I2C_ADDR);
    Wire.write(MLX90614_OBJECT_TEMP);
    
    if (Wire.endTransmission(false) == 0) {
      // Request 3 bytes (2 data bytes + 1 PEC byte)
      Wire.requestFrom(MLX90614_I2C_ADDR, 3);
      
      if (Wire.available() >= 2) {
        // Read temperature data (LSB first)
        uint8_t tempLow = Wire.read();
        uint8_t tempHigh = Wire.read();
        Wire.read(); // Read and discard PEC byte
        
        // Combine bytes into 16-bit value
        uint16_t tempRaw = (tempHigh << 8) | tempLow;
        
        // Convert to Kelvin (resolution is 0.02K per LSB)
        float tempK = tempRaw * 0.02;
        
        // Convert Kelvin to Celsius then to Fahrenheit
        float tempC = tempK - 273.15;
        float tempF = (tempC * 9.0 / 5.0) + 32.0;
        
        // Store as integer in Fahrenheit
        beltTemperature = (int)tempF;
      }
    }
    
    lastBeltTempReadTime = millis();
  }
}

void printData() {
  DEBUG_SERIAL.print("pRpm:");
  DEBUG_SERIAL.print(primaryRPM);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("pTemp:");
  DEBUG_SERIAL.print(primaryTemperature);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("sRpm:");
  DEBUG_SERIAL.print(secondaryRPM);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("sTemp:");
  DEBUG_SERIAL.print(secondaryTemperature);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("beltTemp:");
  DEBUG_SERIAL.println(beltTemperature);
}