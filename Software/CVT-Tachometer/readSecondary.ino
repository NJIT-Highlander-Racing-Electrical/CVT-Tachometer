void readSecondary() {

    // Take an analog reading
  secondaryValue = analogRead(SECONDARY_IR);

  // If the sensor detects the white stripe
  if (secondaryValue > secondaryUpperThreshold) {
    // Mark the current time
    firstSecondaryReadTime = micros();

    // Wait for the reading to go low
    while (analogRead(SECONDARY_IR) > secondaryLowerThreshold) {
      // We aren't interested in doing anything here, but to prevent us from getting stuck, break if it never goes low
      if ((micros() - firstSecondaryReadTime) > timeoutThreshold) {
        secondaryRPM = 0;
        return;
      }
    }

    unsigned long goneLowTime = micros();

    // Now that reading has gone low, wait for it to go high again
    while (analogRead(SECONDARY_IR) < secondaryUpperThreshold) {
      // We aren't interested in doing anything here, but to prevent us from getting stuck, break if it never goes high again
      if ((micros() - goneLowTime) > timeoutThreshold) {
        secondaryRPM = 0;
        return;
      }
    }

    // If we're here, then the second reading was completed
    secondSecondaryReadTime = micros();
    unsigned long goneHighTime = micros();

    // Ensure that secondSecondaryReadTime is greater than firstSecondaryReadTime
    // Since we are using an unsigned long, the micros value will roll over approximately every hour

    if (secondSecondaryReadTime < firstSecondaryReadTime) {
      DEBUG_SERIAL.println("micros() rollover, ignoring reading");
      return;
    }

    // Find elapsed time between first and second reading, then calculate RPM from that
    if ((secondSecondaryReadTime - firstSecondaryReadTime) != 0) {
      secondaryRPM = (1.00 / (float(secondSecondaryReadTime - firstSecondaryReadTime) / 1000000.0)) / numSecondaryTargets * 60.0;
    } else {
      DEBUG_SERIAL.println("readSecondary(): AVOIDED DIVIDE BY ZERO");
    }
  }

    // Before we exit, wait for a low again
    // This way, when we get back to the start of the read secondary function, we do not double count the current HIGH
    // This should execute fairly quickly in regular use, but if it doesn't something is wrong and we should break
    while (analogRead(SECONDARY_IR) > secondaryLowerThreshold) {
      if ((micros() - goneHighTime) > timeoutThreshold) {
        secondaryRPM = 0;
        return;
      }
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

    long total = 0;

    // Average 10 readings
    for (int i = 0; i < 10; i++) {
      total += analogRead(SECONDARY_TEMP);  // Read from the secondary temp pin
    }

    // Calculate the average reading
    secTempReading = total / 10;

    secTempVoltage = secTempReading * 3.3;
    secTempVoltage /= 4095.0;
    secTempVoltage += 0.015;                  // small correction for calibrating to actual temp
    secTempC = (secTempVoltage - 0.5) * 100;  //converting from 10 mv per degree wit 500 mV offset
    secondaryTemperature = (secTempC * 9.0 / 5.0) + 32.0;

    lastSecTempReading = millis();
  }
}