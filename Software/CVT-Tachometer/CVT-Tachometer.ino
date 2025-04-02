/*
*
* This program uses two infrared emitter/receiver pairs to detect revolutions on the primary and
* secondary pulleys of the CVT. The elapsed time between revolutions is used to calculate revolutions
* per minute, and this data is sent over CAN to the rest of the vehicle's subsystems.
*
* There is also a TMP36 temperature sensor on each module that reports ambient temperature inside of the CVT case.
*
* The main core of the ESP32 is dedicated to reading primary RPM, reading primary temperature, and printing data to the Serial Monitor
*
* The second core of the ESP32 is dedicated to reading secondary RPM, reading secondary temperature, and sending data over CAN-Bus
*
* NOTE: This file does not use the shared BajaCAN.h, as it requires the use of both cores
*
* NOTE: Based on the resistor value in the IR photodiode voltage divider, it is possible for the input to the analog pin to exceed 3.3V
* Ensure this does not happen, because if it does, it can interfere with the temperature sensor analog reading or cause damage to hardware
* With a properly set fixed resistor, this should not be an issue, but setting the potentiometer too high would allow the voltage at the
* center tap of the voltage divider to exceed 3.3V.
*
*/

#include "src/libraries/arduino-CAN/src/CAN.h"



// Set DEBUG to false for final program; Serial is just used for troubleshooting
#define DEBUG true
#define DEBUG_SERIAL \
  if (DEBUG) Serial

const int debugPrintInterval = 100;  // Rate at which we print to Serial monitor. This is to reduce calculation issues
int lastPrintTime = 0;               // The last time that we printed to monitor



//#define PRIMARY_IR 34  // Use this definition when using the fixed resistor IR inpu
#define PRIMARY_IR 33    // Use this definition when using the potentiometer IR input
#define PRIMARY_TEMP 35  // Primary TMP36 Sensor pin

//#define SECONDARY_IR 13  // Use this definition when using the fixed resistor IR input
#define SECONDARY_IR 4     // Use this definition when using the potentiometer IR input
#define SECONDARY_TEMP 27  // Secondary TMP36 Sensor pin

// Definitions for all CAN setup parameters
#define CAN_BAUD_RATE 1000E3
#define CAN_TX_GPIO 25
#define CAN_RX_GPIO 26

// Number of milliseconds to wait between transmissions
int canSendInterval = 100;
// Definition to log the last time that a CAN message was sent
int lastCanSendTime = 0;

// CVT Tachometer CAN IDs
const int primaryRPM_ID = 0x01;
const int secondaryRPM_ID = 0x02;
const int primaryTemperature_ID = 0x03;
const int secondaryTemperature_ID = 0x04;
const int statusCVT_ID = 0x5A;

uint8_t statusCVT = 0b11111111;

// Task to run on second core (dual-core processing)
TaskHandle_t Task1;

// Initialize bounds of sensor readings
// These will be continuously updated as the IR sensors gather new data
// The lower third and upper third of this min/max will be used as LOW and HIGH thresholds
int primaryMinReading = 0;
int primaryMaxReading = 0;
int secondaryMinReading = 0;
int secondaryMaxReading = 0;

// Initialize these thresholds at zero; they will be set accordingly later
int primaryLowerThreshold = 0;
int primaryUpperThreshold = 0;
int secondaryLowerThreshold = 0;
int secondaryUpperThreshold = 0;

int primaryValue = 0;  // value read from IR sensor
unsigned long currentPrimaryReadTime = 0;
unsigned long lastPrimaryReadTime = 0;
int primaryRPM = 0;  // calculated RPM value based on elapsed time between readings

int secondaryValue = 0;  // value read from IR sensor
unsigned long currentSecondaryReadTime = 0;
unsigned long lastSecondaryReadTime = 0;
int secondaryRPM = 0;  // calculated RPM value based on elapsed time between readings

// Makes sure that the input goes LOW before counting another revolution
// Prevents double counting of revolution
bool primaryGoneLow = true;
bool secondaryGoneLow = true;

const int timeoutThreshold = 1000;  // If there are no readings in timeoutThreshold milliseconds, reset RPM to zero

const int tempUpdateFrequency = 1000;  // Get a new temperature reading every 1000 milliseconds

unsigned long lastPrimTempReading = 0;  // Variable for last time temperature sensor has been polled
int primTempReading = 0;                // Analog reading from temp sensor
float primTempVoltage = 0;              // Calculated voltage based on analog reading
float primTempC = 0;
float primaryTemperature = 0;

unsigned long lastSecTempReading = 0;  // Variable for last time temperature sensor has been polled
int secTempReading = 0;                // Analog reading from temp sensor
float secTempVoltage = 0;              // Calculated voltage based on analog reading
float secTempC = 0;
float secondaryTemperature = 0;

void setup() {

  CAN.setPins(CAN_RX_GPIO, CAN_TX_GPIO);
  if (!CAN.begin(CAN_BAUD_RATE)) {
    Serial.println("Starting CAN failed!");
    while (1)
      ;
  } else {
    Serial.println("CAN Initialized");
  }


  DEBUG_SERIAL.begin(115200);

  pinMode(PRIMARY_IR, INPUT);
  pinMode(PRIMARY_TEMP, INPUT);
  pinMode(SECONDARY_IR, INPUT);
  pinMode(SECONDARY_TEMP, INPUT);


  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    2000,      /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
  delay(500);

  // Establish a starting point for min and max
  // These will be updated later as new min's and max's are found
  primaryMinReading = analogRead(PRIMARY_IR);
  primaryMaxReading = analogRead(PRIMARY_IR);
  secondaryMinReading = analogRead(SECONDARY_IR);
  secondaryMaxReading = analogRead(SECONDARY_IR);
}




// Main loop
void loop() {

  readPrimary();

  delay(1);  // Delay for stability

  if ((millis() - lastPrintTime) > debugPrintInterval) {
    lastPrintTime = millis();
    printData();
  }
}





// Task 1 executes on secondary core of ESP32 and solely looks at the secondary of the CVT
// All other processing is done on primary core
void Task1code(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    readSecondary();

    delay(1);  // Delay for stability

    if ((millis() - lastCanSendTime) > canSendInterval) {
      lastCanSendTime = millis();
      sendCAN();
    }

    checkStatus();

    checkForRTR();
  }
}
