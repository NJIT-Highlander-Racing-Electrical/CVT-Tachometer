void updatePrimaryRPMs() {

  // If we have not seen a revolution in timeoutThreshold milliseconds, reset the RPM reading to zero
  if ((millis() - lastPrimaryReadTime) > timeoutThreshold) {
    primaryRPM = 0;
  }

  // Update IR sensor reading
  primaryValue = analogRead(primary);

  // If the sensor detects the white stripe and we were not already on the stripe
  if ((primaryValue > primaryUpperThreshold) && primaryGoneLow) {

    // Mark the current time
    currentPrimaryReadTime = millis();

    // Find elapsed time between current reading and previous reading, then calculate RPM from that
    primaryRPM = (1.00 / ((float)(currentPrimaryReadTime - lastPrimaryReadTime) / 1000.0)) * 60.0;

    lastPrimaryReadTime = currentPrimaryReadTime;
    primaryGoneLow = false;

    delayMicroseconds(250);  // delay 250uS for stability
  }

  // We do not want to double count white stripe on same revolution, so wait for black again
  if (primaryValue < primaryLowerThreshold) {
    primaryGoneLow = true;
  }

  // Update upper bound and threshold
  if (primaryValue > primaryMaxReading) {
    updateUpperBound();
  }


  // Update lower bound and threshold
  if (primaryValue < primaryMinReading) {
    updateLowerBound();
  }
  
}



void updateUpperBound() {

  primaryMaxReading = primaryValue;
  // Calculate midpoint
  int minMaxDifference = primaryMaxReading - primaryMinReading;
  // Set threshold to lower third of min and new max
  primaryLowerThreshold = primaryMinReading + (minMaxDifference / 4);
  primaryUpperThreshold = primaryMaxReading - (minMaxDifference / 4);
}

void updateLowerBound() {

  primaryMinReading = primaryValue;
  //Calculate midpoint
  int minMaxDifference = primaryMaxReading - primaryMinReading;
  //Set threshold to midpoint of new min and max
  primaryLowerThreshold = primaryMinReading + (minMaxDifference / 3);
  primaryUpperThreshold = primaryMaxReading - (minMaxDifference / 3);
}