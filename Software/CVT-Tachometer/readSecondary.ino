void readSecondary() {
  // Average a set number of readings before using it for calculations
  long total = 0;
  for (int i = 0; i < numIRSamples; i++) {
    total += analogRead(SECONDARY_IR);
    delayMicroseconds(5);  // let the node recharge to avoid ADC droop
  }
  secondaryValue = total / numIRSamples;

  // Reset RPM if no pulse seen for too long
  if ((millis() - lastSecondaryReadTime) > timeoutThreshold) {
    secondaryRPM = 0;
  }

  // If the sensor detects the white stripe and we were not already on the stripe
  if ((secondaryValue > secondaryUpperThreshold) && secondaryGoneLow) {
    currentSecondaryReadTime = millis();

    if (abs(secondaryUpperThreshold - secondaryLowerThreshold) < 400) {
      DEBUG_SERIAL.println("Not yet calibrated, ignoring secondary reading");
      secondaryRPM = 0;
    } else if ((currentSecondaryReadTime - lastSecondaryReadTime) != 0) {
      secondaryRPM = (1.00 / (float(currentSecondaryReadTime - lastSecondaryReadTime) / 1000.0)) * 60.0 / numSecondaryTargets;
    } else {
      DEBUG_SERIAL.println("readSecondary(): AVOIDED DIVIDE BY ZERO");
    }

    lastSecondaryReadTime = currentSecondaryReadTime;
    secondaryGoneLow = false;
  }

  // Wait for dark section before counting another revolution
  if (secondaryValue < secondaryLowerThreshold) {
    secondaryGoneLow = true;
  }

  // Update thresholds dynamically
  updateSecondaryBounds();

  // Update secondary temperature
  readSecondaryTemp();
}

void updateSecondaryBounds() {
  const float alpha = 0.2;   // Fast adaptation to new peaks
  const float decay = 0.01;  // Slow decay toward current value to prevent stale bounds

  // Update max
  if (secondaryValue > secondaryMaxReading) {
    secondaryMaxReading = (1.0 - alpha) * secondaryMaxReading + alpha * secondaryValue;
  } else {
    secondaryMaxReading = (1.0 - decay) * secondaryMaxReading + decay * secondaryValue;
  }

  // Update min
  if (secondaryValue < secondaryMinReading) {
    secondaryMinReading = (1.0 - alpha) * secondaryMinReading + alpha * secondaryValue;
  } else {
    secondaryMinReading = (1.0 - decay) * secondaryMinReading + decay * secondaryValue;
  }

  // Update thresholds based on new bounds
  int minMaxDifference = secondaryMaxReading - secondaryMinReading;
  secondaryLowerThreshold = secondaryMinReading + (minMaxDifference / 4);
  secondaryUpperThreshold = secondaryMaxReading - (minMaxDifference / 4);
}

void readSecondaryTemp() {
  if ((millis() - lastSecTempReading) > tempUpdateFrequency) {
    long total = 0;

    for (int i = 0; i < 10; i++) {
      total += analogRead(SECONDARY_TEMP);
    }

    secTempReading = total / 10;

    secTempVoltage = secTempReading * 3.3;
    secTempVoltage /= 4095.0;
    secTempC = (secTempVoltage - 0.5) * 100;
    secondaryTemperature = (secTempC * 9.0 / 5.0) + 32.0;
    secondaryTemperature += 17;  // Correct offset based on calibration

    lastSecTempReading = millis();
  }
}
