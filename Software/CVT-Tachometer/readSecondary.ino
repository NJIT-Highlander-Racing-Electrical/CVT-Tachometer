void readSecondary() {

  // Average a set number of readings before using it for calculations
  long total = 0;
  for (int i = 0; i < numIRSamples; i++) {
    total += analogRead(SECONDARY_IR);
  }
  secondaryValue = total / numIRSamples;

  // If we haven't seen a reading in timeoutThreshold milliseconds, reset to zero
  if ((millis() - lastSecondaryReadTime) > timeoutThreshold) {
    secondaryRPM = 0;
  }

  // If the sensor detects the white stripe and we were not already on the stripe
  if ((secondaryValue > secondaryUpperThreshold) && secondaryGoneLow) {
    // Mark the current time
    currentSecondaryReadTime = millis();

    // Find elapsed time between current reading and previous reading, then calculate RPM from that
    if (abs(secondaryUpperThreshold - secondaryLowerThreshold) < 400) {
      DEBUG_SERIAL.println("Not yet calibrated, ignoring secondary reading");
      // If we do not see a significant difference in the two thresholds, we have not completed a full revolution.
      // Thus, we should ignore the calculation so we do not display/save erroneous
      secondaryRPM = 0;
    } else if ((currentSecondaryReadTime - lastSecondaryReadTime) != 0) {
      if (!ignoreNextSecondaryReading) {  // We only ignore when we just printed to CAN and delayed (for watchdog) and we may have missed a reading
        secondaryRPM = (1.00 / (float(currentSecondaryReadTime - lastSecondaryReadTime) / 1000.0)) * 60.0 / numSecondaryTargets;
      } else {
        ignoreNextSecondaryReading = false;  // If this boolean was true, we can disable it and continue on so we calculate next time
      }
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
    secondaryLowerThreshold = secondaryMinReading + (minMaxDifference / 4);
    secondaryUpperThreshold = secondaryMaxReading - (minMaxDifference / 4);
  }

  // Update lower bound and threshold
  if (secondaryValue < secondaryMinReading) {
    secondaryMinReading = secondaryValue;
    //Calculate midpoint
    int minMaxDifference = secondaryMaxReading - secondaryMinReading;
    // Set new thresholds
    secondaryLowerThreshold = secondaryMinReading + (minMaxDifference / 4);
    secondaryUpperThreshold = secondaryMaxReading - (minMaxDifference / 4);
  }
}


void readSecondaryTemp() {
  if ((millis() - lastSecTempReading) > tempUpdateFrequency) {

    long total = 0;

    // Average 10 readings
    for (int i = 0; i < 10; i++) {
      total += analogRead(SECONDARY_TEMP);  // Read from the secondary temp pin
    }

    // Calculate the average reading
    secTempReading = total / 10;

    secTempVoltage = secTempReading * 3.3;
    secTempVoltage /= 4095.0;
    secTempVoltage += 0.03;                   // small correction for calibrating to actual temp
    secTempC = (secTempVoltage - 0.5) * 100;  //converting from 10 mv per degree wit 500 mV offset
    secondaryTemperature = (secTempC * 9.0 / 5.0) + 32.0;

    lastSecTempReading = millis();
  }
}