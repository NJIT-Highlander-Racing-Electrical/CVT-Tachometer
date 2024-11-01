void printData() {

  DEBUG_SERIAL_PRIMARY.print("primLowerThreshold:");
  DEBUG_SERIAL_PRIMARY.print(primaryLowerThreshold);
  DEBUG_SERIAL_PRIMARY.print(", ");

  DEBUG_SERIAL_PRIMARY.print("primUpperThreshold:");
  DEBUG_SERIAL_PRIMARY.print(primaryUpperThreshold);
  DEBUG_SERIAL_PRIMARY.print(", ");

  DEBUG_SERIAL_PRIMARY.print("primValue:");
  DEBUG_SERIAL_PRIMARY.print(primaryValue);
  DEBUG_SERIAL_PRIMARY.print(", ");

  DEBUG_SERIAL_PRIMARY.print("primRPMAverage:");
  DEBUG_SERIAL_PRIMARY.print(primaryTotal / primaryCount);
  DEBUG_SERIAL_PRIMARY.print(", ");

  DEBUG_SERIAL_SECONDARY.print("secLowerThreshold:");
  DEBUG_SERIAL_SECONDARY.print(secondaryLowerThreshold);
  DEBUG_SERIAL_SECONDARY.print(", ");

  DEBUG_SERIAL_SECONDARY.print("secUpperThreshold:");
  DEBUG_SERIAL_SECONDARY.print(secondaryUpperThreshold);
  DEBUG_SERIAL_SECONDARY.print(", ");

  DEBUG_SERIAL_SECONDARY.print("secValue:");
  DEBUG_SERIAL_SECONDARY.print(secondaryValue);
  DEBUG_SERIAL_SECONDARY.print(", ");

  DEBUG_SERIAL_SECONDARY.print("secRPM:");
  DEBUG_SERIAL_SECONDARY.print(secondaryRPM);
  DEBUG_SERIAL_SECONDARY.print(", ");

  DEBUG_SERIAL_GENERAL.print("Temperature_F:");
  DEBUG_SERIAL_GENERAL.println(temperatureF);
}