void updateRPMs() {

  sampleStartTime = millis();
  primarySampleRevs = 0;
  secondarySampleRevs = 0;

  while ((millis() - sampleStartTime < sampleInterval)) {

    //Serial.print("PRIMARY VALUE:");
    //Serial.println(analogRead(primary));

    primaryValue = analogRead(primary);

    if ((primaryValue > primaryThreshold) && primaryGoneLow) {
      primarySampleRevs++;
      //Serial.println("primarySampleRevs++");
      primaryGoneLow = false;
    }
    if (primaryValue < primaryThreshold) primaryGoneLow = true;


    //Serial.print("SECONDARY VALUE:");
    //Serial.println(analogRead(secondary));
    secondaryValue = analogRead(secondary);

    if ((secondaryValue > secondaryThreshold) && secondaryGoneLow) {
      secondarySampleRevs++;
      //Serial.println("secondarySampleRevs++");
      secondaryGoneLow = false;
    }
    if (secondaryValue < secondaryThreshold) secondaryGoneLow = true;


    delayMicroseconds(500);
  }

  // Sample period has ended here; report on collected data
  // RPM = (Revolutions/Sample Periods (ms) * milliseconds per minute) = Revolutions Per Minute

  primaryRPM = primarySampleRevs / sampleInterval * 60000;

  secondaryRPM = secondarySampleRevs / sampleInterval * 60000;
}