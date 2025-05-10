void printData() {

  int plt = primaryLowerThreshold;
  int put = primaryUpperThreshold;
  int pv = primaryValue;
  int pr = primaryRPM;
  int ptf = primaryTemperature;

  int slt = secondaryLowerThreshold;
  int sut = secondaryUpperThreshold;
  int sv = secondaryValue;
  int sr = secondaryRPM;
  int stf = secondaryTemperature;


  DEBUG_SERIAL.print("pLowThresh:");
  DEBUG_SERIAL.print(plt);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("pUpperThresh:");
  DEBUG_SERIAL.print(put);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("pVal:");
  DEBUG_SERIAL.print(pv);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("pRpm:");
  DEBUG_SERIAL.print(pr);
  DEBUG_SERIAL.print(", ");
  /*
  DEBUG_SERIAL.print("pTemp:");
  DEBUG_SERIAL.print(ptf);
  DEBUG_SERIAL.print(", ");
  */

  DEBUG_SERIAL.print("sLowThresh:");
  DEBUG_SERIAL.print(slt);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("sUpperThresh:");
  DEBUG_SERIAL.print(sut);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("sVal:");
  DEBUG_SERIAL.print(sv);
  DEBUG_SERIAL.print(", ");

  DEBUG_SERIAL.print("sRpm:");
  DEBUG_SERIAL.println(sr);
  /*
  
  DEBUG_SERIAL.print("sTemp:");
  DEBUG_SERIAL.println(stf);
  */
<<<<<<< HEAD
}
=======
}
>>>>>>> 3e557d17067eebd249ee6d81547dedd6824b76d6
