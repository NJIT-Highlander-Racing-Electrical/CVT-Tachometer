/*
* CVT Tachometer with IR sensors and temperature monitoring
* Fixed timing issues, race conditions, and improved filtering
*/

#include "BajaCAN.h"
#include <Wire.h>

// Set DEBUG to false for final program
#define DEBUG true

#define DEBUG_SERIAL \
  if (DEBUG) Serial

#define PRIMARY_IR 32    // Primary Comparator Output
#define PRIMARY_TEMP 33  // Primary TMP36 Sensor pin

#define SECONDARY_IR 27    // Secondary Comparator Output
#define SECONDARY_TEMP 14  // Secondary TMP36 Sensor pin

// MLX90614 I2C Configuration
#define MLX90614_I2C_ADDR 0x5A
#define MLX90614_OBJECT_TEMP 0x07
#define I2C_SDA 21
#define I2C_SCL 22

// USER-DEFINED PARAMETERS
const int debugPrintInterval = 100;
const int tempUpdateFrequency = 500;
const int numPrimaryTargets = 8;
const int numSecondaryTargets = 3;

// Timeout if no pulse in 1 second (1,000,000 microseconds)
const unsigned long timeoutThreshold = 1000000;

// Debounce time - adjusted for realistic RPM ranges
// Primary: 8 targets, max ~4000 RPM = 533 pulses/sec = 1875us between pulses
// Secondary: 3 targets, max ~4000 RPM = 200 pulses/sec = 5000us between pulses
// Set debounce to 1500us to allow fast speeds while rejecting noise
const unsigned long debounceTimeMicros = 1500;

// Maximum reasonable RPM to filter out noise spikes
const int MAX_PRIMARY_RPM = 5000;
const int MAX_SECONDARY_RPM = 5000;

// END USER PARAMETERS

// Serial Print timing
int lastPrintTime = 0;

// Primary timing variables (all volatile for ISR safety)
volatile unsigned long currentPrimaryReadTime = 0;
volatile unsigned long lastPrimaryReadTime = 0;
volatile bool primaryUpdateFlag = false;

// Secondary timing variables
volatile unsigned long currentSecondaryReadTime = 0;
volatile unsigned long lastSecondaryReadTime = 0;
volatile bool secondaryUpdateFlag = false;

// Temperature timing
unsigned long lastPrimTempReadTime = 0;
unsigned long lastSecTempReadTime = 0;
unsigned long lastBeltTempReadTime = 0;

// State flags
bool primaryIsFirstReading = true;
bool secondaryIsFirstReading = true;
bool primaryIgnoreNext = false;
bool secondaryIgnoreNext = false;

// Moving average filter (median filter for better spike rejection)
const int RPM_FILTER_SIZE = 5;
int primaryRPMBuffer[RPM_FILTER_SIZE] = {0};
int secondaryRPMBuffer[RPM_FILTER_SIZE] = {0};
int primaryBufferIndex = 0;
int secondaryBufferIndex = 0;
int primaryBufferCount = 0;  // Track how many samples we have
int secondaryBufferCount = 0;

// Function Prototypes
void primaryISR();
void secondaryISR();
void readPrimaryTemp();
void readSecondaryTemp();
void readBeltTemp();
void checkPrimaryTimeout();
void checkSecondaryTimeout();
int calculateMedianRPM(int* buffer, int size);
void printData();

void setup() {
  Serial.begin(460800);
  delay(100);

  // Initialize I2C for MLX90614
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize CAN - send every 25ms (40Hz)
  setupCAN(CVT, 25);

  // Configure pins
  pinMode(PRIMARY_IR, INPUT);
  pinMode(PRIMARY_TEMP, INPUT);
  pinMode(SECONDARY_IR, INPUT);
  pinMode(SECONDARY_TEMP, INPUT);

  // Initialize timing to current time
  unsigned long now = micros();
  currentPrimaryReadTime = now;
  lastPrimaryReadTime = now;
  currentSecondaryReadTime = now;
  lastSecondaryReadTime = now;

  // Attach interrupts on rising edge
  attachInterrupt(digitalPinToInterrupt(PRIMARY_IR), primaryISR, RISING);
  attachInterrupt(digitalPinToInterrupt(SECONDARY_IR), secondaryISR, RISING);

  DEBUG_SERIAL.println("CVT Tachometer Initialized");
  DEBUG_SERIAL.print("Primary targets: ");
  DEBUG_SERIAL.println(numPrimaryTargets);
  DEBUG_SERIAL.print("Secondary targets: ");
  DEBUG_SERIAL.println(numSecondaryTargets);
  DEBUG_SERIAL.print("Debounce time (us): ");
  DEBUG_SERIAL.println(debounceTimeMicros);
}

void loop() {
  // Check for timeouts
  checkPrimaryTimeout();
  checkSecondaryTimeout();

  // Process primary RPM calculation
  if (primaryUpdateFlag) {
    // Capture timing atomically
    noInterrupts();
    unsigned long capturedCurrent = currentPrimaryReadTime;
    unsigned long capturedLast = lastPrimaryReadTime;
    primaryUpdateFlag = false;
    interrupts();

    // Skip first reading - need two points to calculate
    if (primaryIsFirstReading) {
      noInterrupts();
      lastPrimaryReadTime = capturedCurrent;
      interrupts();
      primaryIsFirstReading = false;
      primaryIgnoreNext = false;
      DEBUG_SERIAL.println("Primary: First reading captured");
    }
    // If recovering from timeout, re-establish baseline
    else if (primaryIgnoreNext) {
      noInterrupts();
      lastPrimaryReadTime = capturedCurrent;
      interrupts();
      primaryIgnoreNext = false;
      DEBUG_SERIAL.println("Primary: Baseline re-established after timeout");
    }
    // Normal calculation
    else {
      unsigned long elapsed = capturedCurrent - capturedLast;
      
      if (elapsed > 0) {
        // Calculate RPM: (60,000,000 microseconds/min) / elapsed / targets
        int calculatedRPM = (60000000.0 / elapsed) / numPrimaryTargets;

        // Sanity check
        if (calculatedRPM > 0 && calculatedRPM < MAX_PRIMARY_RPM) {
          // Add to buffer
          primaryRPMBuffer[primaryBufferIndex] = calculatedRPM;
          primaryBufferIndex = (primaryBufferIndex + 1) % RPM_FILTER_SIZE;
          if (primaryBufferCount < RPM_FILTER_SIZE) {
            primaryBufferCount++;
          }
          
          // Calculate filtered value
          primaryRPM = calculateMedianRPM(primaryRPMBuffer, primaryBufferCount);
          
          // Update last reading time atomically
          noInterrupts();
          lastPrimaryReadTime = capturedCurrent;
          interrupts();
        } else {
          DEBUG_SERIAL.print("Primary RPM rejected: ");
          DEBUG_SERIAL.println(calculatedRPM);
        }
      }
    }
  }

  // Process secondary RPM calculation
  if (secondaryUpdateFlag) {
    // Capture timing atomically
    noInterrupts();
    unsigned long capturedCurrent = currentSecondaryReadTime;
    unsigned long capturedLast = lastSecondaryReadTime;
    secondaryUpdateFlag = false;
    interrupts();

    // Skip first reading - need two points to calculate
    if (secondaryIsFirstReading) {
      noInterrupts();
      lastSecondaryReadTime = capturedCurrent;
      interrupts();
      secondaryIsFirstReading = false;
      secondaryIgnoreNext = false;
      DEBUG_SERIAL.println("Secondary: First reading captured");
    }
    // If recovering from timeout, re-establish baseline
    else if (secondaryIgnoreNext) {
      noInterrupts();
      lastSecondaryReadTime = capturedCurrent;
      interrupts();
      secondaryIgnoreNext = false;
      DEBUG_SERIAL.println("Secondary: Baseline re-established after timeout");
    }
    // Normal calculation
    else {
      unsigned long elapsed = capturedCurrent - capturedLast;
      
      if (elapsed > 0) {
        // Calculate RPM
        int calculatedRPM = (60000000.0 / elapsed) / numSecondaryTargets;

        // Sanity check
        if (calculatedRPM > 0 && calculatedRPM < MAX_SECONDARY_RPM) {
          // Add to buffer
          secondaryRPMBuffer[secondaryBufferIndex] = calculatedRPM;
          secondaryBufferIndex = (secondaryBufferIndex + 1) % RPM_FILTER_SIZE;
          if (secondaryBufferCount < RPM_FILTER_SIZE) {
            secondaryBufferCount++;
          }
          
          // Calculate filtered value
          secondaryRPM = calculateMedianRPM(secondaryRPMBuffer, secondaryBufferCount);
          
          // Update last reading time atomically
          noInterrupts();
          lastSecondaryReadTime = capturedCurrent;
          interrupts();
        } else {
          DEBUG_SERIAL.print("Secondary RPM rejected: ");
          DEBUG_SERIAL.println(calculatedRPM);
        }
      }
    }
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
 * FUNCTION DEFINITIONS
 */

void primaryISR() {
  unsigned long now = micros();
  
  // Debounce: ignore pulses too close together
  unsigned long timeSinceLast = now - currentPrimaryReadTime;
  if (timeSinceLast < debounceTimeMicros) {
    return;  // Noise/bounce, ignore
  }
  
  // Update timing
  lastPrimaryReadTime = currentPrimaryReadTime;
  currentPrimaryReadTime = now;
  primaryUpdateFlag = true;
}

void secondaryISR() {
  unsigned long now = micros();
  
  // Debounce: ignore pulses too close together
  unsigned long timeSinceLast = now - currentSecondaryReadTime;
  if (timeSinceLast < debounceTimeMicros) {
    return;  // Noise/bounce, ignore
  }
  
  // Update timing
  lastSecondaryReadTime = currentSecondaryReadTime;
  currentSecondaryReadTime = now;
  secondaryUpdateFlag = true;
}

void checkPrimaryTimeout() {
  unsigned long currentTime = micros();
  unsigned long timeSinceLastPulse = currentTime - currentPrimaryReadTime;
  
  if (timeSinceLastPulse > timeoutThreshold) {
    if (primaryRPM != 0) {  // Only update if not already zero
      primaryRPM = 0;
      primaryIgnoreNext = true;  // Ignore next pulse to re-establish baseline
      
      // Clear the buffer
      for (int i = 0; i < RPM_FILTER_SIZE; i++) {
        primaryRPMBuffer[i] = 0;
      }
      primaryBufferCount = 0;
      primaryBufferIndex = 0;
      
      DEBUG_SERIAL.println("Primary timeout - RPM set to 0");
    }
  }
}

void checkSecondaryTimeout() {
  unsigned long currentTime = micros();
  unsigned long timeSinceLastPulse = currentTime - currentSecondaryReadTime;
  
  if (timeSinceLastPulse > timeoutThreshold) {
    if (secondaryRPM != 0) {  // Only update if not already zero
      secondaryRPM = 0;
      secondaryIgnoreNext = true;  // Ignore next pulse to re-establish baseline
      
      // Clear the buffer
      for (int i = 0; i < RPM_FILTER_SIZE; i++) {
        secondaryRPMBuffer[i] = 0;
      }
      secondaryBufferCount = 0;
      secondaryBufferIndex = 0;
      
      DEBUG_SERIAL.println("Secondary timeout - RPM set to 0");
    }
  }
}

int calculateMedianRPM(int* buffer, int count) {
  if (count == 0) return 0;
  
  // Create temporary array for sorting
  int sortedBuffer[RPM_FILTER_SIZE];
  memcpy(sortedBuffer, buffer, count * sizeof(int));
  
  // Bubble sort
  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      if (sortedBuffer[j] > sortedBuffer[j + 1]) {
        int temp = sortedBuffer[j];
        sortedBuffer[j] = sortedBuffer[j + 1];
        sortedBuffer[j + 1] = temp;
      }
    }
  }
  
  // Return median value
  return sortedBuffer[count / 2];
}

void readPrimaryTemp() {
  if ((millis() - lastPrimTempReadTime) > tempUpdateFrequency) {
    long total = 0;

    // Average 10 readings
    for (int i = 0; i < 10; i++) {
      total += analogRead(PRIMARY_TEMP);
    }

    int primTempReading = total / 10;
    float primTempVoltage = (float)primTempReading * 3.3 / 4095.0;
    float primTempC = (primTempVoltage - 0.5) * 100;
    primaryTemperature = (primTempC * 9.0 / 5.0) + 32.0;

    lastPrimTempReadTime = millis();
  }
}

void readSecondaryTemp() {
  if ((millis() - lastSecTempReadTime) > tempUpdateFrequency) {
    long total = 0;

    // Average 10 readings
    for (int i = 0; i < 10; i++) {
      total += analogRead(SECONDARY_TEMP);
    }

    int secTempReading = total / 10;
    float secTempVoltage = (float)secTempReading * 3.3 / 4095.0;
    float secTempC = (secTempVoltage - 0.5) * 100;
    secondaryTemperature = (secTempC * 9.0 / 5.0) + 32.0;

    lastSecTempReadTime = millis();
  }
}

void readBeltTemp() {
  if ((millis() - lastBeltTempReadTime) > tempUpdateFrequency) {
    Wire.beginTransmission(MLX90614_I2C_ADDR);
    Wire.write(MLX90614_OBJECT_TEMP);
    
    if (Wire.endTransmission(false) == 0) {
      Wire.requestFrom(MLX90614_I2C_ADDR, 3);
      
      if (Wire.available() >= 2) {
        uint8_t tempLow = Wire.read();
        uint8_t tempHigh = Wire.read();
        Wire.read(); // Discard PEC byte
        
        uint16_t tempRaw = (tempHigh << 8) | tempLow;
        float tempK = tempRaw * 0.02;
        float tempC = tempK - 273.15;
        float tempF = (tempC * 9.0 / 5.0) + 32.0;
        
        beltTemperature = (int)tempF;
      }
    }
    
    lastBeltTempReadTime = millis();
  }
}

void printData() {
  DEBUG_SERIAL.print("pRpm:");
  DEBUG_SERIAL.print(primaryRPM);
  DEBUG_SERIAL.print(", pTemp:");
  DEBUG_SERIAL.print(primaryTemperature);
  DEBUG_SERIAL.print(", sRpm:");
  DEBUG_SERIAL.print(secondaryRPM);
  DEBUG_SERIAL.print(", sTemp:");
  DEBUG_SERIAL.print(secondaryTemperature);
  DEBUG_SERIAL.print(", beltTemp:");
  DEBUG_SERIAL.println(beltTemperature);
}