void readPrimary() {

  // Take an analog reading
  primaryValue = analogRead(PRIMARY_IR);

  // If the sensor detects the white stripe
  if (primaryValue > primaryUpperThreshold) {
    // Mark the current time
    firstPrimaryReadTime = micros();

    // Wait for the reading to go low
    while (analogRead(PRIMARY_IR) > primaryLowerThreshold) {
      // We aren't interested in doing anything here, but to prevent us from getting stuck, break if it never goes low
      if ((micros() - firstPrimaryReadTime) > timeoutThreshold) {
        primaryRPM = 0;
        return;
      }
    }

    unsigned long goneLowTime = micros();

    // Now that reading has gone low, wait for it to go high again
    while (analogRead(PRIMARY_IR) < primaryUpperThreshold) {
      // We aren't interested in doing anything here, but to prevent us from getting stuck, break if it never goes high again
      if ((micros() - goneLowTime) > timeoutThreshold) {
        primaryRPM = 0;
        return;
      }
    }

    // If we're here, then the second reading was completed
    secondPrimaryReadTime = micros();
    unsigned long goneHighTime = micros();

    // Ensure that secondPrimaryReadTime is greater than firstPrimaryReadTime
    // Since we are using an unsigned long, the micros value will roll over approximately every hour

    if (secondPrimaryReadTime < firstPrimaryReadTime) {
      DEBUG_SERIAL.println("micros() rollover, ignoring reading");
      return;
    }

    // Find elapsed time between first and second reading, then calculate RPM from that
    if ((secondPrimaryReadTime - firstPrimaryReadTime) != 0) {
      primaryRPM = (1.00 / (float(secondPrimaryReadTime - firstPrimaryReadTime) / 1000000.0)) / numPrimaryTargets * 60.0;
    } else {
      DEBUG_SERIAL.println("readPrimary(): AVOIDED DIVIDE BY ZERO");
    }
  }

    // Before we exit, wait for a low again
    // This way, when we get back to the start of the read primary function, we do not double count the current HIGH
    // This should execute fairly quickly in regular use, but if it doesn't something is wrong and we should break
    while (analogRead(PRIMARY_IR) > primaryLowerThreshold) {
      if ((micros() - goneHighTime) > timeoutThreshold) {
        primaryRPM = 0;
        return;
      }
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

    long total = 0;

    // Average 10 readings
    for (int i = 0; i < 10; i++) {
      total += analogRead(PRIMARY_TEMP);  // Read from the primary temp pin
    }

    // Calculate the average reading
    primTempReading = total / 10;


    primTempVoltage = primTempReading * 3.3;
    primTempVoltage /= 4095.0;
    primTempVoltage += 0.07;                    // small correction for calibrating to actual temp
    primTempC = (primTempVoltage - 0.5) * 100;  //converting from 10 mv per degree wit 500 mV offset
    primaryTemperature = (primTempC * 9.0 / 5.0) + 32.0;

    lastPrimTempReading = millis();
  }
}