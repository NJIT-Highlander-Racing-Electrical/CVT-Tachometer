void printData() {

  int plt = primaryLowerThreshold;
  int put = primaryUpperThreshold;
  int pv = primaryValue;
  int pr = primaryRPM;
  int ptf = primTempF;

  int slt = secondaryLowerThreshold;
  int sut = secondaryUpperThreshold;
  int sv = secondaryValue;
  int sr = secondaryRPM;
  int stf = secTempF;


  DEBUG_SERIAL.print("primLowerThreshold:");
  DEBUG_SERIAL.print(plt);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("primUpperThreshold:");
  DEBUG_SERIAL.print(put);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("primValue:");
  DEBUG_SERIAL.print(pv);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("primRPM:");
  DEBUG_SERIAL.print(pr);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("primTemp F:");
  DEBUG_SERIAL.print(ptf);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("secLowerThreshold:");
  DEBUG_SERIAL.print(slt);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("secUpperThreshold:");
  DEBUG_SERIAL.print(sut);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("secValue:");
  DEBUG_SERIAL.print(sv);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("secRPM:");
  DEBUG_SERIAL.print(sr);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("secTemp F:");
  DEBUG_SERIAL.println(stf);
}