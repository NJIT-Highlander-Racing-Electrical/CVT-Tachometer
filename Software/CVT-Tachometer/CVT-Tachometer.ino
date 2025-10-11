/*
*
* This program uses two infrared emitter/receiver pairs with comparators to detect revolutions on the primary and
* secondary pulleys of the CVT. The analog reading from the infrared receiver configuration (which acts as a variable
* resistor, so it is wired as a voltage divider) is passed through a comparator with a known reference. Then, this comparator
* output is set up as an interrupt input to the ESP32, and each CVT reading will trigger the ISR. Calculations are done to
* determine the RPM based on the elapsed time between readings, and this data is sent over CAN to the rest of the vehicle's subsystems.
*
* There is also a TMP36 temperature sensor on each module that reports ambient temperature inside of the CVT case.
*
*/

#include "BajaCAN.h"

// Set DEBUG to false for final program; Serial is just used for troubleshooting
#define DEBUG false

#define DEBUG_SERIAL \
  if (DEBUG) Serial

#define PRIMARY_IR 32    // Primary Comparator Output
#define PRIMARY_TEMP 33  // Primary TMP36 Sensor pin

#define SECONDARY_IR 27    // Secondary Comparator Output
#define SECONDARY_TEMP 14  // Secondary TMP36 Sensor pin

// USER-DEFINED PARAMETERS
const int debugPrintInterval = 100;   // Rate at which we print to Serial monitor
const int tempUpdateFrequency = 500;  // Get a new temperature reading every 500 milliseconds
const int numPrimaryTargets = 2;      // Number of independent sensing targets on CVT primary
const int numSecondaryTargets = 2;    // Number of independent sensing targets on CVT secondary

const int timeoutThreshold = 1000000;  // If there are no readings in timeoutThreshold microseconds, reset RPM to zero

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

// Update flags (set in ISR)
volatile bool primaryUpdateFlag = false;
volatile bool secondaryUpdateFlag = false;

// Timeout flags; these get set if the readings timed out so we ignore the first calculation back
bool primaryTimedOut = false;
bool secondaryTimedOut = false;


// Function Prototypes
void primaryISR();
void readPrimaryTemp();
void checkPrimaryTimeout();

void readSecondaryTemp();
void secondaryISR();
void checkSecondaryTimeout();

void printData();

void setup() {
  Serial.begin(460800);

  // Initialize CAN using BajaCAN. We want to send every 25 milliseconds (40Hz send rate)
  setupCAN(CVT, 25);

  // Configure pins
  pinMode(PRIMARY_IR, INPUT);
  pinMode(PRIMARY_TEMP, INPUT);
  pinMode(SECONDARY_IR, INPUT);
  pinMode(SECONDARY_TEMP, INPUT);

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
        primaryRPM = (60000000.0 / elapsed) / numPrimaryTargets;
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
        secondaryRPM = (60000000.0 / elapsed) / numSecondaryTargets;
      }
    }

    secondaryTimedOut = false;
  }

  // Read temperatures
  readPrimaryTemp();
  readSecondaryTemp();

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
  lastPrimaryReadTime = currentPrimaryReadTime;
  currentPrimaryReadTime = micros();
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
  lastSecondaryReadTime = currentSecondaryReadTime;
  currentSecondaryReadTime = micros();
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
    float secTempC = (secTempVoltage - 0.5) * 100;   // converting from 10 mv per degree with 500 mV offset
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
  DEBUG_SERIAL.println(secondaryTemperature);
}
