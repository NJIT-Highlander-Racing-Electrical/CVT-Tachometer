double primStartTime = 0, secStartTime = 0;
int primPointer = 0, secPointer = 0;
bool primUpdate = false, secUpdate = false;
bool primFirst = true, secFirst = true;

void updateRPMs() {

  /*sampleStartTime = millis();
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
  }*/


      //Serial.print("PRIMARY VALUE:");
    //Serial.println(analogRead(primary));



/*
    primaryValue = analogRead(primary);
    if (primUpdate) {
      if (!primFirst)
        primRecords[primPointer] = millis() - primStartTime;

      if ((primaryValue > primaryThreshold) && primaryGoneLow) {
        if (primFirst) {
          primRecords[primPointer] = millis() - primStartTime;
          primFirst = false;
        }

        primStartTime = millis();

        //Serial.println("primarySampleRevs++");
        primaryGoneLow = false;

        primPointer += 1;
        if (primPointer == 10) {
          primPointer = 0;
        }
      }

      if (millis() - primStartTime > 10000) {
        for (int i = 0; i < 10; i++)
          primRecords[i] = 99999;

        primUpdate = false;
      }
    } else if ((primaryValue > primaryThreshold) && primaryGoneLow) {
      primUpdate = true;
      primStartTime = millis();
      primaryGoneLow = false;
      primFirst = true;
    }
    if (primaryValue < primaryThreshold) primaryGoneLow = true;



    secondaryValue = analogRead(secondary);
    if (secUpdate) {
      if (!secFirst)
        secRecords[secPointer] = millis() - secStartTime;

      if ((secondaryValue > secondaryThreshold) && secondaryGoneLow) {
        if (secFirst) {
          secRecords[secPointer] = millis() - secStartTime;
          secFirst = false;
        }

        secStartTime = millis();

        //Serial.println("primarySampleRevs++");
        secondaryGoneLow = false;

        secPointer += 1;
        if (secPointer == 10) {
          secPointer = 0;
        }
      }

      if (millis() - secStartTime > 10000) {
        for (int i = 0; i < 10; i++)
          secRecords[i] = 99999;

        secUpdate = false;
      }
    } else if ((secondaryValue > secondaryThreshold) && secondaryGoneLow) {
      secUpdate = true;
      secStartTime = millis();
      secondaryGoneLow = false;
      secFirst = true;
    }
    if (secondaryValue < secondaryThreshold) secondaryGoneLow = true;

  // Sample period has ended here; report on collected data
  // RPM = (Revolutions/Sample Periods (ms) * milliseconds per minute) = Revolutions Per Minute

  //primaryRPM = primarySampleRevs / sampleInterval * 60000;
  double averageTime = 0;
  int d = 0;
  for (int i = 0; i < 10; i++) {
    if (primRecords[i] != 99999) {
      averageTime += primRecords[i];
      d += 1;
    }
  }

  if (averageTime == 0) {
    primaryRPM = 0;
  } else {
    primaryRPM = 60 * (1000.0 / (averageTime / d));
  }


  averageTime = 0;
  d = 0;
  for (int i = 0; i < 10; i++) {
    if (secRecords[i] != 99999) {
      averageTime += secRecords[i];
      d += 1;
    }
  }

  if (averageTime == 0) {
    secondaryRPM = 0;
  } else {
    secondaryRPM = 60 * (1000.0 / (averageTime / d));
  }

  */




  unsigned long startTime = millis();
  int i = 0;
  bool timeout = false;
  while (primaryValue < primaryThreshold && !timeout) {
    primaryValue = analogRead(primary);
    if (millis() - startTime > 200) {
      timeout = true;
    }
  }
  primaryGoneLow = false;

  if (!timeout) {
    startTime = millis();
    while ((millis() - startTime) < 200 && i < 2) {
      primaryValue = analogRead(primary);

      // Update upper bound and threshold
      if (primaryValue > primaryMaxReading) {
        primaryMaxReading = primaryValue;
        // Calculate midpoint
        int minMaxDifference = primaryMaxReading - primaryMinReading;
        // Set threshold to midpoint of min and new max
        primaryThreshold = primaryMinReading + (minMaxDifference/2);
      }
      
      // Update lower bound and threshold
      if (primaryValue < primaryMinReading) {
        primaryMinReading = primaryValue;
        //Calculate midpoint
        int minMaxDifference = primaryMaxReading - primaryMinReading;
        //Set threshold to midpoint of new min and max
        primaryThreshold = primaryMinReading + (minMaxDifference/2);
      }

      if ((primaryValue > primaryThreshold) && primaryGoneLow) {
        i += 1;
        primaryGoneLow = false;
      }

      if (primaryValue < primaryThreshold) primaryGoneLow = true;
    }

    if (i != 0) {
      primaryRPM = (1000.0 / ((millis() - startTime) / (float)i)) * 60.0;
    }
    else
      primaryRPM = 0;
  } else
    primaryRPM = 0;



  primaryRPM = (primaryRPM > 5000) ? 5000 : primaryRPM * .75;
}
