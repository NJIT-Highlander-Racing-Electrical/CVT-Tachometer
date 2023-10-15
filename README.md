 # CVT-Tachometer

A subsystem that measures the RPM of the primary and secondary on the CVT. This subsystem also has a temperature sensor to detect if the CVT is overheating.

## Hardware:

* IR Sensors that point at primary and secondary.
- These can detect when a white stripe on the CVT passes by (one revolution)
* Microcontroller to convert the number of revolutions per second into an RPM value for dashboard and DAS (via CAN-Bus)
  - Uses ESP32 and TJA1051T CAN-Bus transceiver
* TMP36 Temperature Sensor to look at ambient temperature in the CVT case
- Sends a bit via CAN-Bus to turn on an overheating status LED on the dashboard

 ## Notes

 * Super-Bright IR LEDs must be powered via a transistor. Their output current is 100mA and an ESP32 can only output 40mA via its GPIO pins
 * Using standard infrared photodiodes, we can simply power the IR leds without needing a 38kHz pulse rate
 * Connections from the LEDs/sensors to the main body should be sealed/waterproof
 * The holes in the CVT case should be 20mm
