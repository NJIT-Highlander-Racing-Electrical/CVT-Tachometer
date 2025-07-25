void readPrimary() {

  // Average a set number of readings before using it for calculations
  long total = 0;
  for (int i = 0; i < numIRSamples; i++) {
    total += analogRead(PRIMARY_IR);
      delayMicroseconds(5);  // let the node recharge
  }
  primaryValue = total / numIRSamples;


  // If we haven't seen a reading in timeoutThreshold milliseconds, reset to zero
  if ((millis() - lastPrimaryReadTime) > timeoutThreshold) {
    primaryRPM = 0;
  }

  // If the sensor detects the white stripe and we were not already on the stripe
  if ((primaryValue > primaryUpperThreshold) && primaryGoneLow) {
    // Mark the current time
    currentPrimaryReadTime = millis();

    // Find elapsed time between current reading and previous reading, then calculate RPM from that
    if (abs(primaryUpperThreshold - primaryLowerThreshold) < 400) {
      DEBUG_SERIAL.println("Not yet calibrated, ignoring primary reading");
      // If we do not see a significant difference in the two thresholds, we have not completed a full revolution.
      // Thus, we should ignore the calculation so we do not display/save erroneous
      primaryRPM = 0;
    } else if ((currentPrimaryReadTime - lastPrimaryReadTime) != 0) {
      primaryRPM = (1.00 / (float(currentPrimaryReadTime - lastPrimaryReadTime) / 1000.0)) * 60.0 / numPrimaryTargets;
    } else {
      DEBUG_SERIAL.println("readPrimary(): AVOIDED DIVIDE BY ZERO");
    }

    lastPrimaryReadTime = currentPrimaryReadTime;
    primaryGoneLow = false;
  }

  // We do not want to double count white stripe on same revolution, so wait for black again
  if (primaryValue < primaryLowerThreshold) {
    primaryGoneLow = true;
  }

  // Update threshold based on min/max readings
  updatePrimaryBounds();

  // Update temperature if necessary
  readPrimaryTemp();
}


void updatePrimaryBounds() {
   const float alpha = 0.2;   // How fast to move toward a *new* extreme
  const float decay = 0.01;  // How fast to decay toward the current value when no new extreme

  // Update max: move toward a new high OR decay down slowly
  if (primaryValue > primaryMaxReading) {
    primaryMaxReading = (1.0 - alpha) * primaryMaxReading + alpha * primaryValue;
  } else {
    primaryMaxReading = (1.0 - decay) * primaryMaxReading + decay * primaryValue;
  }

  // Update min: move toward a new low OR decay up slowly
  if (primaryValue < primaryMinReading) {
    primaryMinReading = (1.0 - alpha) * primaryMinReading + alpha * primaryValue;
  } else {
    primaryMinReading = (1.0 - decay) * primaryMinReading + decay * primaryValue;
  }

  // Recalculate thresholds
  int minMaxDifference = primaryMaxReading - primaryMinReading;
  primaryLowerThreshold = primaryMinReading + (minMaxDifference / 4);
  primaryUpperThreshold = primaryMaxReading - (minMaxDifference / 4);

}


void readPrimaryTemp() {

  if ((millis() - lastPrimTempReading) > tempUpdateFrequency) {

    long total = 0;

    // Average 10 readings
    for (int i = 0; i < 10; i++) {
      total += analogRead(PRIMARY_TEMP);  // Read from the primary temp pin
    }

    // Calculate the average reading
    primTempReading = total / 10;


    primTempVoltage = primTempReading * 3.3;
    primTempVoltage /= 4095.0;
    primTempC = (primTempVoltage - 0.5) * 100;  //converting from 10 mv per degree wit 500 mV offset
    primaryTemperature = (primTempC * 9.0 / 5.0) + 32.0;
    primaryTemperature += 17;  // correct temperature because it is offset roughly this amount from physical testing

    lastPrimTempReading = millis();
  }
}