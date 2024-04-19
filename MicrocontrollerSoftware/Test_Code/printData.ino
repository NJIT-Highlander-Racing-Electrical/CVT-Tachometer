int count = 0;

void printData() {
  //if (count >= 10) {
    /*Serial.print("{");
    for (int i = 0; i < 10; i++) {
      Serial.print(primRecords[i]); 
      if (i < 9)
        Serial.print(", ");
    }

    Serial.print("}");

    Serial.println(primaryValue);*/

  DEBUG_SERIAL.print("primaryRPM:");
  DEBUG_SERIAL.print(primaryRPM);
  DEBUG_SERIAL.print(",");
  DEBUG_SERIAL.print("secondaryRPM:");
  DEBUG_SERIAL.print(secondaryRPM);
  DEBUG_SERIAL.print(",");
  DEBUG_SERIAL.print("Temperature (F):");
  DEBUG_SERIAL.println(temperatureF);

  //DEBUG_SERIAL.println(primaryValue);
  //DEBUG_SERIAL.println(secondaryValue);

  count = 0;
  //}

  count += 1;
}