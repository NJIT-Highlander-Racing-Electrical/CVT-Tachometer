 # CVT-Tachometer

A subsystem that measures the RPM of the primary and secondary on the CVT. This subsystem also has a temperature sensor to detect if the CVT is overheating.

## Hardware Components:

* IR Sensors that point at primary and secondary.
- These can detect when a white stripe on the CVT passes by (one revolution)
* Microcontroller to convert the number of revolutions per second into an RPM value for dashboard and DAS (via CAN-Bus)
  - Uses ESP32 and TJA1051T CAN-Bus transceiver
* TMP36 Temperature Sensor to look at ambient temperature in the CVT case
- Sends a bit via CAN-Bus to turn on an overheating status LED on the dashboard

 ## Notes

 * Super-Bright IR LEDs support an output current of 100mA with a forward voltage of 1.6V. We are using a 22 ohm resistor for a current of roughly 80 mA
 * Using standard infrared photodiodes, we can simply power the IR leds without needing a 38kHz pulse rate
 * Connections from the LEDs/sensors to the main body should be sealed/waterproof
 * The holes in the CVT case should be 20mm
 * The infrared diode appears to act like a variable resistor, but voltage polarity must be + on short leg and - on long leg

 * The software utilizes the dual core functionality of the ESP32
   * Core 1 reads primaryRPM and deals with CAN-Bus communication
   * Core 0 solely reads secondaryRPM


## Electrical Wiring Notes

### Ethernet Cable Wire Colors
* Orange: IR Emitter +
* Orange/White: IR Emitter -
* Blue: IR Receiver +
* Blue/White: IR Receiver -
* Green: TMP36 OUT
* Brown/White: TMP36 3.3V VCC
* Brown: TMP36 GND
