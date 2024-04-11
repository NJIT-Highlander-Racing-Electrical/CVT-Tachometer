void printData() {
  Serial.print("primaryRPM:");
  Serial.print(primaryRPM);
  Serial.print(",");
  Serial.print("secondaryRPM:");
  Serial.print(secondaryRPM);
  Serial.print(",");
  Serial.print("Temperature (F):");
  Serial.println(temperatureF);
}