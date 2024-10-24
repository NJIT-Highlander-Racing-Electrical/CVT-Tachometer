void printData() {

  DEBUG_SERIAL.print("primLowerThreshold:");
  DEBUG_SERIAL.print(primaryLowerThreshold);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("primUpperThreshold:");
  DEBUG_SERIAL.print(primaryUpperThreshold);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("primValue:");
  DEBUG_SERIAL.print(primaryValue);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("primRPMAverage:");
  DEBUG_SERIAL.print(primaryTotal / primaryCount);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("secLowerThreshold:");
  DEBUG_SERIAL.print(secondaryLowerThreshold);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("secUpperThreshold:");
  DEBUG_SERIAL.print(secondaryUpperThreshold);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("secValue:");
  DEBUG_SERIAL.print(secondaryValue);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("secRPM:");
  DEBUG_SERIAL.print(secondaryRPM);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("Temperature_F:");
  DEBUG_SERIAL.println(temperatureF);
}