//EXTREMELY IMPORTANT NOTE: THE CAN FREQUENCY ON THIS IS NORMAL 500E3), BUT IT IS "DOUBLED" ON SOME OTHER BOARDS (1000E3) BECAUSE THEY ARE MESSED UP

#define DEBUG true
#define DEBUG_SERIAL if(DEBUG)Serial

//Note: these were just for testing at home; use the real values below
int primaryThreshold = 2500;
int secondaryThreshold = 2500;

#include <CAN.h>
#define TX_GPIO_NUM 21
#define RX_GPIO_NUM 22

float sampleInterval = 500;  //SAMPLE HOWEVER MANY REVS IN X MILLISECONDS
unsigned long sampleStartTime = 0;

const int primary = 36;  // IR Sensor input pin value
int primaryValue = 0;    // value read from IR sensor
//int primaryThreshold = 700;  // Above this IR input threshold is considered a HIGH
int primarySampleRevs = 0;
int primaryRPM = 0;

const int secondary = 39;  // IR Sensor input pin value
int secondaryValue = 0;    // value read from IR sensor
//int secondaryThreshold = 700;  // Above this IR input threshold is considered a HIGH
int secondarySampleRevs = 0;
int secondaryRPM = 0;

double primRecords[10] = {99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999};
double secRecords[10] = {99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999};

//Boring HIGH/LOW stuff
//Essentially makes sure that the input goes low before counting another revolution
bool primaryGoneLow = true;
bool secondaryGoneLow = true;

int tempSensor = 34;
int reading = 0;
float voltage = 0;
float temperatureC = 0;
float temperatureF = 0;

double lastSend = 0;


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
}

int i = 0, m = 1;
void loop() {

  //DEBUG_SERIAL.println("Updating RPMs...");
  updateRPMs();
  
  /*primaryRPM = i;
  i += m * 1;
  if (i > 4500)
    m = -1;
  else if (i <= 0)
    m = 1;*/
  //DEBUG_SERIAL.println("Updating Temp...");
  updateTemp();
  printData();

  if (millis() - lastSend > 25) {
    canSender();
    lastSend = millis();
  }
}


void canSender() {
  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  //DEBUG_SERIAL.print("Sending PRIMARY RPM ... ");

  CAN.beginPacket(0x1F);  //sets the ID
  CAN.print((primaryRPM < 0) ? 0 : primaryRPM / 10);  //prints data to CAN Bus just like Serial.print
  CAN.endPacket();

  delay(5);

  CAN.beginPacket(0x20);
  CAN.print((secondaryRPM < 0) ? 0 : secondaryRPM / 10);
  CAN.endPacket();

  //delay(10);

  /*CAN.beginPacket(0x21);
  CAN.print(temperatureF);
  CAN.endPacket();

  delay(5);*/
}
