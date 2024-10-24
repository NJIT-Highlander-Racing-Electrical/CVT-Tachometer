void updatePrimaryRPMs() {

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
    primaryRPM = (1.00 / (float(currentPrimaryReadTime - lastPrimaryReadTime) / 1000.0)) * 60.0;

    if (primaryRPM < 5000)  {
      addPrimaryReading(primaryRPM);
    }

    lastPrimaryReadTime = currentPrimaryReadTime;
    primaryGoneLow = false;

    delay(1); // delay 1 ms for stability

  }

  // We do not want to double count white stripe on same revolution, so wait for black again
  if (primaryValue < primaryLowerThreshold) {
    primaryGoneLow = true;
  }

  //
  // Update threshold based on min/max readings
  //

  // Update upper bound and threshold
  if (primaryValue > primaryMaxReading) {
    primaryMaxReading = primaryValue;
    // Calculate midpoint
    int minMaxDifference = primaryMaxReading - primaryMinReading;
    // Set threshold to lower third of min and new max
    primaryLowerThreshold = primaryMinReading + (minMaxDifference / 4);
    primaryUpperThreshold = primaryMaxReading - (minMaxDifference / 4);
  }

  // Update lower bound and threshold
  if (primaryValue < primaryMinReading) {
    primaryMinReading = primaryValue;
    //Calculate midpoint
    int minMaxDifference = primaryMaxReading - primaryMinReading;
    //Set threshold to midpoint of new min and max
    primaryLowerThreshold = primaryMinReading + (minMaxDifference / 3);
    primaryUpperThreshold = primaryMaxReading - (minMaxDifference / 3);
  }
}

void addPrimaryReading(float newReading) {
    primaryTotal -= readings[primaryIndex]; // Remove the oldest reading from total
    readings[primaryIndex] = newReading; // Add the new reading
    primaryTotal += newReading; // Add the new reading to total

    if (primaryCount < primaryNumReadings) {
        primaryCount++; // Increase count until we have enough readings
    }

    primaryIndex = (primaryIndex + 1) % primaryNumReadings; // Move to the next index
}
