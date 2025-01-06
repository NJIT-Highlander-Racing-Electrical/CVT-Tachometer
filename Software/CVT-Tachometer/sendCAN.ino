void sendCAN() {

  CAN.beginPacket(statusCVT_ID);
  CAN.print(1);
  CAN.endPacket();

  CAN.beginPacket(primaryRPM_ID);
  CAN.print(primaryRPM);
  CAN.endPacket();

  CAN.beginPacket(secondaryRPM_ID);
  CAN.print(secondaryRPM);
  CAN.endPacket();

  CAN.beginPacket(primaryTemperature_ID);
  CAN.print(primaryTemperature);
  CAN.endPacket();

  CAN.beginPacket(secondaryTemperature_ID);
  CAN.print(secondaryTemperature);
  CAN.endPacket();
}