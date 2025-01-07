void checkForRTR() {

  // Check if a packet has been received
  // Returns the packet size in bytes or 0 if no packet received
  int packetSize = CAN.parsePacket();
  int packetId;

  if ((packetSize || CAN.packetId() != -1) && (packetSize != 0)) {
    // received a packet
    packetId = CAN.packetId();  // Get the packet ID

    // If this packet was a RTR and we are the proper recipient, return the requested data
    if (CAN.packetRtr() && (packetId == statusCVT_ID)) {
      CAN.beginPacket(statusCVT_ID);
      CAN.print(statusCVT);
      CAN.endPacket();
    }
  }
}