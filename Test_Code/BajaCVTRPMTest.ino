#include <SPI.h>
#include "SdFat.h"
SdFat SD;
File myFile;

const int sdChipSelect = 10;
const int analogInPin = A6;
const int analogInPin2 = A7;

bool alreadyAddedPrimary = false;
bool alreadyAddedSecondary = false;

int primaryValue = 0;        // value read from the pot
int secondaryValue = 0;

int revolutionCountPrimary = 0;
int revolutionCountSecondary = 0;
unsigned long primaryMillisecondsPerPass;
unsigned long secondaryMillisecondsPerPass;
unsigned long primaryRPM;
unsigned long secondaryRPM;

unsigned long primaryArray[105];
unsigned long secondaryArray[105];
int arrayPosition = 0;

/////////////////////////////////////// CHANGEABLE VARIABLES /////////////////////////////

int samplePeriod = 250;
int samplesBeforeSDSave = 10;

//////////////////////////////////////////////////////////////////////////////////////////

unsigned long startMillis = 0;
unsigned long currentMillis = 0;

int primaryThreshold = 100;
int secondaryThreshold = 50;

void setup() {
  Serial.begin(115200);

  if (!SD.begin(sdChipSelect)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  SD.remove("test.txt");

  myFile = SD.open("test.txt", FILE_WRITE);

  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  myFile = SD.open("test.txt", FILE_WRITE);


}

void loop() {

  // Serial.print("Primary Revolution Count: ");
  // Serial.println(revolutionCountPrimary);

  primaryValue = analogRead(analogInPin);

  //Serial.print("Primary Value: ");
  //Serial.println(primaryValue);

  if ((primaryValue > primaryThreshold) && (alreadyAddedPrimary == false)) {

    revolutionCountPrimary = (revolutionCountPrimary + 1);
    alreadyAddedPrimary = true;

  }

  if (primaryValue < primaryThreshold) {
    alreadyAddedPrimary = false;
  }



  //Serial.print("Secondary Revolution Count: ");
  //Serial.println(revolutionCountSecondary);

  secondaryValue = analogRead(analogInPin2);

  //Serial.print("Secondary Value: ");
  //Serial.println(secondaryValue);

  if ((secondaryValue > secondaryThreshold) && (alreadyAddedSecondary == false)) {

    revolutionCountSecondary = (revolutionCountSecondary + 1);
    alreadyAddedSecondary = true;

  }

  if (secondaryValue < secondaryThreshold) {
    alreadyAddedSecondary = false;


  }

  /*

    Serial.println();
    Serial.println();

    Serial.print("Primary Analog Value: ");
    Serial.println(analogRead(analogInPin));

    Serial.print("Secondary Analog Value: ");
    Serial.println(analogRead(analogInPin2));

    Serial.println();
    Serial.println();

  */

  if ((millis() - startMillis) > samplePeriod) {

    primaryMillisecondsPerPass = (samplePeriod / revolutionCountPrimary);
    primaryRPM = 30000 / primaryMillisecondsPerPass;
    Serial.print(primaryRPM);
    Serial.print(",");
    myFile.print(primaryRPM);
    myFile.print(" ");

    secondaryMillisecondsPerPass = (samplePeriod / revolutionCountSecondary);
    secondaryRPM = 30000 / secondaryMillisecondsPerPass;
    Serial.println(secondaryRPM);

    myFile.println(secondaryRPM);

    arrayPosition = (arrayPosition + 1);
    //Serial.print("Array Position: ");
    //Serial.println(arrayPosition);

    if (arrayPosition >= samplesBeforeSDSave) {

      myFile.flush();
      //Serial.println("SD CLOSED!");

      arrayPosition = 0;

    }


    //Reset timer and counters

    startMillis = millis();
    revolutionCountPrimary = 0;
    revolutionCountSecondary = 0;

  }


}














// OLD CODE


/*
  if (millis() - secondaryStartMillis > samplePeriod) {
    secondaryMillisecondsPerPass = (samplePeriod / secondaryCounter);
    secondaryRPM = 30000 / secondaryMillisecondsPerPass;
    Serial.print("secondaryRPM: ");
    Serial.println(secondaryRPM);

    secondaryArray[secondaryArrayPosition] = secondaryRPM;
    secondaryArrayPosition = secondaryArrayPosition++;

    if (secondaryArrayPosition >= 100) {

      Serial.print("Last 100 Secondary Readings: ");
      for (int i = 0; i < 101; i++) Serial.println(secondaryArrayPosition[i]);
      // Print last 100 readings to SD Card Here

      secondaryArrayPosition = 0;

    }


    secondaryStartMillis = millis();
    secondaryCounter = 0;
  }


*/
