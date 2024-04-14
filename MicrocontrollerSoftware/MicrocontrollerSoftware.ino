//EXTREMELY IMPORTANT NOTE: THE CAN FREQUENCY ON THIS IS NORMAL 500E3), BUT IT IS "DOUBLED" ON SOME OTHER BOARDS (1000E3) BECAUSE THEY ARE MESSED UP

//Note: these were just for testing at home; use the real values below
int primaryThreshold = 3000;
int secondaryThreshold = 3000;

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

//Boring HIGH/LOW stuff
//Essentially makes sure that the input goes low before counting another revolution
bool primaryGoneLow = true;
bool secondaryGoneLow = true;

int tempSensor = 34;
int reading = 0;
float voltage = 0;
float temperatureC = 0;
float temperatureF = 0;

int updateFrequency = 5000;
unsigned long lastUpdateTime = 0;


int rpmSamplePeriod = 75;
unsigned long rpmSampleStart = 0;

void setup() {
  Serial.begin(115200);
  pinMode(tempSensor, INPUT);

  CAN.setPins(RX_GPIO_NUM, TX_GPIO_NUM);
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1)
      ;
  } else {
    Serial.println("CAN Initialized");
  }
}

void loop() {

  //Serial.println("Updating RPMs...");
  rpmSampleStart = millis();
  while ((millis() - rpmSampleStart) < rpmSamplePeriod) {
    updateRPMs();
  }
  //Serial.println("Updating Temp...");
  if ((millis() - lastUpdateTime) > updateFrequency) {
    lastUpdateTime = millis();
    updateTemp();
  }

  updateCanbus();

  printData();

  delay(1);
}

void updateCanbus() {

  CAN.beginPacket(0x1F);  //sets the ID
  CAN.print(primaryRPM);  //prints data to CAN Bus just like Serial.print
  CAN.endPacket();

  delay(5);

  CAN.beginPacket(0x20);
  CAN.print(secondaryRPM);
  CAN.endPacket();
  delay(5);
}