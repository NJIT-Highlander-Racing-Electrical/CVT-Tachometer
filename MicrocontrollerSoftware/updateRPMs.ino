unsigned long lastPrimaryReadTime = 0;
unsigned long currentPrimaryReadTime = 0;
unsigned long primaryElapsedTime = 999999999999;

unsigned long lastSecondaryReadTime = 0;
unsigned long currentSecondaryReadTime = 0;
unsigned long secondaryElapsedTime = 999999999999;


const int rpmTimeout = 1000;  // If no readings from a sensor in one second, set to zero


void updateRPMs() {

  primaryValue = analogRead(primary);
  Serial.print("PRIMARY VALUE:");
  Serial.println(primaryValue);

  if ((primaryValue > primaryThreshold) && primaryGoneLow) {
    currentPrimaryReadTime = millis();
    primaryElapsedTime = currentPrimaryReadTime - lastPrimaryReadTime + 1;  // to prevent divide by zero errors
    lastPrimaryReadTime = currentPrimaryReadTime;
    primaryGoneLow = false;
  }
  if (primaryValue < primaryThreshold) primaryGoneLow = true;




  secondaryValue = analogRead(secondary);
  Serial.print("SECONDARY VALUE:");
  Serial.println(secondaryValue);

  if ((secondaryValue > secondaryThreshold) && secondaryGoneLow) {
    currentSecondaryReadTime = millis();
    secondaryElapsedTime = currentSecondaryReadTime - lastSecondaryReadTime + 1;  // to prevent divide by zero errors
    lastSecondaryReadTime = currentSecondaryReadTime;
    secondaryGoneLow = false;
  }
  if (secondaryValue < secondaryThreshold) secondaryGoneLow = true;



  if ((millis() - lastPrimaryReadTime) > rpmTimeout) primaryRPM = 0;
  else primaryRPM = 60000 / primaryElapsedTime;

  if ((millis() - lastSecondaryReadTime) > rpmTimeout) secondaryRPM = 0;
  else secondaryRPM = 60000 / secondaryElapsedTime;

  delayMicroseconds(250);
}