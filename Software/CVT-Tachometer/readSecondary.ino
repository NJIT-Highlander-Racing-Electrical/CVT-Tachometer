void readSecondary() {

  // Get analog reading
  secondaryValue = analogRead(SECONDARY_IR);

  // If we haven't seen a reading in timeoutThreshold milliseconds, reset to zero
  if ((millis() - lastSecondaryReadTime) > timeoutThreshold) {
    secondaryRPM = 0;
  }

  // If the sensor detects the white stripe and we were not already on the stripe
  if ((secondaryValue > secondaryUpperThreshold) && secondaryGoneLow) {
    // Mark the current time
    currentSecondaryReadTime = millis();

    // Find elapsed time between current reading and previous reading, then calculate RPM from that
    if ((currentSecondaryReadTime - lastSecondaryReadTime) != 0) {

      secondaryRPM = (1.00 / (float(currentSecondaryReadTime - lastSecondaryReadTime) / 1000.0)) * 60.0;
    } else {
      DEBUG_SERIAL.println("readSecondary(): AVOIDED DIVIDE BY ZERO");
    }

    lastSecondaryReadTime = currentSecondaryReadTime;
    secondaryGoneLow = false;

  }

  // We do not want to double count white stripe on same revolution, so wait for black again
  if (secondaryValue < secondaryLowerThreshold) {
    secondaryGoneLow = true;
  }

  // Update threshold based on min/max readings
  updateSecondaryBounds();

  // Update temperature if necessary
  readSecondaryTemp();
}


void updateSecondaryBounds() {
  // Update upper bound and threshold
  if (secondaryValue > secondaryMaxReading) {
    secondaryMaxReading = secondaryValue;
    // Calculate midpoint
    int minMaxDifference = secondaryMaxReading - secondaryMinReading;
    // Set new thresholds
    secondaryLowerThreshold = secondaryMinReading + (minMaxDifference / 3);
    secondaryUpperThreshold = secondaryMaxReading - (minMaxDifference / 3);
  }

  // Update lower bound and threshold
  if (secondaryValue < secondaryMinReading) {
    secondaryMinReading = secondaryValue;
    //Calculate midpoint
    int minMaxDifference = secondaryMaxReading - secondaryMinReading;
    // Set new thresholds
    secondaryLowerThreshold = secondaryMinReading + (minMaxDifference / 3);
    secondaryUpperThreshold = secondaryMaxReading - (minMaxDifference / 3);
  }
}


void readSecondaryTemp() {
  if ((millis() - lastSecTempReading) > tempUpdateFrequency) {

    secTempReading = analogRead(SECONDARY_TEMP);
    secTempVoltage = secTempReading * 3.3;
    secTempVoltage /= 4095.0;
    secTempC = (secTempVoltage - 0.5) * 100;  //converting from 10 mv per degree wit 500 mV offset
    secondaryTemperature = (secTempC * 9.0 / 5.0) + 32.0;
    
    lastSecTempReading = millis();
  }



}