void readPrimary() {

  // Get analog reading
  primaryValue = analogRead(PRIMARY_IR);

  // If we haven't seen a reading in timeoutThreshold milliseconds, reset to zero
  if ((millis() - lastPrimaryReadTime) > timeoutThreshold) {
    primaryRPM = 0;
  }

  // If the sensor detects the white stripe and we were not already on the stripe
  if ((primaryValue > primaryUpperThreshold) && primaryGoneLow) {
    // Mark the current time
    currentPrimaryReadTime = millis();

    // Find elapsed time between current reading and previous reading, then calculate RPM from that
    if ((currentPrimaryReadTime - lastPrimaryReadTime) != 0) {
    primaryRPM = (1.00 / (float(currentPrimaryReadTime - lastPrimaryReadTime) / 1000.0)) * 60.0;
    }
    else {
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
  // Update upper bound and threshold
  if (primaryValue > primaryMaxReading) {
    primaryMaxReading = primaryValue;
    // Calculate midpoint
    int minMaxDifference = primaryMaxReading - primaryMinReading;
    // Set new thresholds
    primaryLowerThreshold = primaryMinReading + (minMaxDifference / 4);
    primaryUpperThreshold = primaryMaxReading - (minMaxDifference / 4);
  }

  // Update lower bound and threshold
  if (primaryValue < primaryMinReading) {
    primaryMinReading = primaryValue;
    //Calculate midpoint
    int minMaxDifference = primaryMaxReading - primaryMinReading;
    // Set new thresholds
    primaryLowerThreshold = primaryMinReading + (minMaxDifference / 3);
    primaryUpperThreshold = primaryMaxReading - (minMaxDifference / 3);
  }
}


void readPrimaryTemp() {
 
  if ((millis() - lastPrimTempReading) > tempUpdateFrequency) {

    primTempReading = analogRead(PRIMARY_TEMP);
    primTempVoltage = primTempReading * 3.3;
    primTempVoltage /= 4095.0;
    primTempC = (primTempVoltage - 0.5) * 100;  //converting from 10 mv per degree wit 500 mV offset
    primaryTemperature = (primTempC * 9.0 / 5.0) + 32.0;

    lastPrimTempReading = millis();
  }
}