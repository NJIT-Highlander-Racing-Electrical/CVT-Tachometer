/*
*
* This program uses two infrared emitter/receiver pairs to detect revolutions on the primary and
* secondary pulleys of the CVT. The elapsed time between revolutions is used to calculate revolutions
* per minute, and this data is sent over CAN to the rest of the vehicle's subsystems.
* There is also a TMP36 temperature sensor that reports ambient temperature inside of the CVT case.
* On the primary core of the ESP32, several tasks are done, such as reading the primary's revolutions,
* sending data over CAN, and printing data to the Serial Monitor if in debug mode. The secondary core
* is solely responsible for reading the revolutions of the secondary.
*
*
* Important Note: THE CAN FREQUENCY ON THIS IS NORMAL 500E3 kbps, BUT IT IS "DOUBLED" ON SOME OTHER BOARDS (1000E3) BECAUSE THEY ARE MESSED UP
* This is a common issue among some newer variants of the ESP32 dev boards.
*
*/

#include <CAN.h>
#define TX_GPIO_NUM 21 // CAN TX pin
#define RX_GPIO_NUM 22 // CAN RX pin
#define canSendFreq 25 // Number of CAN messages to be sent per second (in regular intervals)

// Set DEBUG to false for final program; Serial is just used for troubleshooting
#define DEBUG true
#define DEBUG_SERIAL \
  if (DEBUG) Serial

#define primary 36 // IR Sensor input pin value
#define secondary 39  // IR Sensor input pin value
#define tempSensor 34 // Temp sensor GPIO pin

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

int primaryValue = 0;    // value read from IR sensor
unsigned long currentPrimaryReadTime = 2; // Initialized to 2 to prevent initial divide by zero error
unsigned long lastPrimaryReadTime = 1; // Initialized to 1 to prevent initial divide by zero error
int primaryRPM = 0; // calculated RPM value based on elapsed time between readings

int secondaryValue = 0;    // value read from IR sensor
unsigned long currentSecondaryReadTime = 2; // Initialized to 2 to prevent initial divide by zero error
unsigned long lastSecondaryReadTime = 1; // Initialized to 1 to prevent initial divide by zero error
int secondaryRPM = 0; // calculated RPM value based on elapsed time between readings

const int timeoutThreshold = 1000; // If there are no readings in timeoutThreshold milliseconds, reset RPM to zero

// Makes sure that the input goes LOW before counting another revolution
// Prevents double counting of revolution
bool primaryGoneLow = true;
bool secondaryGoneLow = true;

int reading = 0; // Analog reading from temp sensor
float voltage = 0; // Calculated voltage based on analog reading
float temperatureC = 0;
float temperatureF = 0;

double lastSend = 0; // millis() reading of last CAN message send; use to send CAN messages at regular interval
int canSendDelay = 1000/canSendFreq; // Calculated millisecond delay between CAN messages based off of canSendFreq;


void setup() {
  DEBUG_SERIAL.begin(115200);

  pinMode(tempSensor, INPUT);

  CAN.setPins(RX_GPIO_NUM, TX_GPIO_NUM);
  if (!CAN.begin(500E3)) {
    DEBUG_SERIAL.println("Starting CAN failed!");
    while (1)
      ;
  } else {
    DEBUG_SERIAL.println("CAN Initialized");
  }



  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
  delay(500);

  // Establish a starting point for min and max
  // These will be updated later as new min's and max's are found
  primaryMinReading = analogRead(primary);
  primaryMaxReading = analogRead(primary);
  secondaryMinReading = analogRead(secondary);
  secondaryMaxReading = analogRead(secondary);
}

// Task 1 executes on secondary core of ESP32 and solely looks at the secondary of the CVT
// All other processing is done on primary core
void Task1code(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

if ((millis() - lastSecondaryReadTime) > timeoutThreshold) {
    secondaryRPM = 0;
  }

    // Update IR sensor reading
    secondaryValue = analogRead(secondary);

    // If the sensor detects the white stripe and we were not already on the stripe
    if ((secondaryValue > secondaryUpperThreshold) && secondaryGoneLow) {
      // Mark the current time 
      currentSecondaryReadTime = millis();

      // Find elapsed time between current reading and previous reading, then calculate RPM from that
      secondaryRPM = (1.00 / (float(currentSecondaryReadTime - lastSecondaryReadTime) / 1000.0)) * 60.0;

      // Maybe add some smoothing/filtering here to average readings
      // Or a timeout so RPM returns to zero

      lastSecondaryReadTime = currentSecondaryReadTime;
      secondaryGoneLow = false;
    }

    // We do not want to double count white stripe on same revolution, so wait for black again
    if (secondaryValue < secondaryLowerThreshold) {
      secondaryGoneLow = true;
    }



    //
    // Update threshold based on min/max readings
    //

    // Update upper bound and threshold
    if (secondaryValue > secondaryMaxReading) {
      secondaryMaxReading = secondaryValue;
      // Calculate midpoint
      int minMaxDifference = secondaryMaxReading - secondaryMinReading;
      // Set threshold to upper third of min and new max
      secondaryLowerThreshold = secondaryMinReading + (minMaxDifference / 3);
      secondaryUpperThreshold = secondaryMaxReading - (minMaxDifference / 3);
    }

    // Update lower bound and threshold
    if (secondaryValue < secondaryMinReading) {
      secondaryMinReading = secondaryValue;
      //Calculate midpoint
      int minMaxDifference = secondaryMaxReading - secondaryMinReading;
      //Set threshold to lower third of new min and max
      secondaryLowerThreshold = secondaryMinReading + (minMaxDifference / 3);
      secondaryUpperThreshold = secondaryMaxReading - (minMaxDifference / 3);
    }
  }
}


// Main loop
void loop() {

  updatePrimaryRPMs();
  updateTemp();
  printData();

  if (millis() - lastSend > canSendDelay) {
    canSender();
    lastSend = millis();
  }
}


void canSender() {
  // send packet: id is 11 bits, packet can contain up to 8 bytes of data

  CAN.beginPacket(0x1F);                              //sets the ID
  CAN.print((primaryRPM < 0) ? 0 : primaryRPM / 10);  //prints data to CAN Bus just like Serial.print
  CAN.endPacket();

  // Delay between packets to not overload CAN bus with two immediate messages
  delay(5);

  CAN.beginPacket(0x20);
  CAN.print((secondaryRPM < 0) ? 0 : secondaryRPM / 10);
  CAN.endPacket();

  //delay(5);

  /*CAN.beginPacket(0x21);
  CAN.print(temperatureF);
  CAN.endPacket();

  delay(5);*/
}
