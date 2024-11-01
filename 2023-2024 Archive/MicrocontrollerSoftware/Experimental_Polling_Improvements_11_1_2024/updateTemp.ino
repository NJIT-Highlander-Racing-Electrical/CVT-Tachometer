void updateTemp() {
  reading = analogRead(tempSensor);
  voltage = reading * 3.3;
  voltage /= 4095.0;
  voltage += 0.12;  //this shouldn't be needed, but the ADC of the ESP32 is off by a decent amount
  //Serial.print(voltage);
  //Serial.println(" volts");
  temperatureC = (voltage - 0.5) * 100;  //converting from 10 mv per degree wit 500 mV offset
  //Serial.print(temperatureC);
  //Serial.println(" degrees C");
  temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;

}