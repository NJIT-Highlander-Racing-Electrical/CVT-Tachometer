float sampleInterval = 250;  //SAMPLE HOWEVER MANY REVS IN X MILLISECONDS
unsigned long sampleStartTime = 0;

const int primary = 36;      // IR Sensor input pin value
int primaryValue = 0;        // value read from IR sensor
int primaryThreshold = 700;  // Above this IR input threshold is considered a HIGH
int primarySampleRevs = 0;
int primaryRPM = 0;

const int secondary = 39;      // IR Sensor input pin value
int secondaryValue = 0;        // value read from IR sensor
int secondaryThreshold = 700;  // Above this IR input threshold is considered a HIGH
int secondarySampleRevs = 0;
int secondaryRPM = 0;

//Boring HIGH/LOW stuff
//Essentially makes sure that the input goes low before counting another revolution
bool primaryGoneLow = true;
bool secondaryGoneLow = true;

int tempSensor = 34;


void setup() {
  Serial.begin(115200);
  pinMode(tempSensor, INPUT);
}

void loop() {

  sampleStartTime = millis();
  primarySampleRevs = 0;
  secondarySampleRevs = 0;

  while ((millis() - sampleStartTime < sampleInterval)) {

    //Serial.print("PRIMARY VALUE:");
    //Serial.println(analogRead(primary));

    primaryValue = analogRead(primary);

    if ((primaryValue > primaryThreshold) && primaryGoneLow) {
      primarySampleRevs++;
      //Serial.println("primarySampleRevs++");
      primaryGoneLow = false;
    }
    if (primaryValue < primaryThreshold) primaryGoneLow = true;


    //Serial.print("SECONDARY VALUE:");
    //Serial.println(analogRead(secondary));
    secondaryValue = analogRead(secondary);

    if ((secondaryValue > secondaryThreshold) && secondaryGoneLow) {
      secondarySampleRevs++;
      //Serial.println("secondarySampleRevs++");
      secondaryGoneLow = false;
    }
    if (secondaryValue < secondaryThreshold) secondaryGoneLow = true;


    delayMicroseconds(500);
  }


  //Sample period has ended here; report on collected data
  // RPM = (Revolutions/Sample Periods (ms) * milliseconds per minute) = Revolutions Per Minute

  //PRIMARY SERIAL

  primaryRPM = primarySampleRevs / sampleInterval * 60000;



  //SECONDARY SERIAL

  secondaryRPM = secondarySampleRevs / sampleInterval * 60000;




  //TEMPERATURE SENSOR SERIAL

  analogRead(tempSensor);
  delay(15);
  int reading = analogRead(tempSensor);
  float voltage = reading * 3.3;
  voltage /= 4095.0;
  voltage += 0.12;  //this shouldn't be needed, but the ADC of the ESP32 is off by a decent amount
  //Serial.print(voltage);
  //Serial.println(" volts");
  float temperatureC = (voltage - 0.5) * 100;  //converting from 10 mv per degree wit 500 mV offset
  //Serial.print(temperatureC);
  //Serial.println(" degrees C");
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;


  Serial.print("primaryRPM:");
  Serial.print(primaryRPM);
  Serial.print(",");
  Serial.print("secondaryRPM:");
  Serial.print(secondaryRPM);
  Serial.print(",");
  Serial.print("Temperature (F):");
  Serial.println(temperatureF);



  //Serial.println();

  delay(1);
}