 # CVT-Tachometer

A subsystem that measures the RPM of the primary and secondary on the CVT. This subsystem also has a temperature sensor to detect if the CVT is overheating.


## 2024-2025 Design Goals

* Everything last year seemed to work well, but a few improvements to the existing system could be made:
  * Software - smoothing/averaging can be implemented to make the dashboard displays more visually appealing
  * Sensor Enclosures - the sensor housings should be made out of something other than PLA
  * Wiring - the wiring from the sensors to the main electronics housing were tough to do. A small PCB for the electronics with a connector/cable would be nicer than hand soldering wires.

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

### Previous Issues 
 * One issue noticed during the testing phase was that the analog sensor readings were constantly peaking (4095). The resistor divider for the IR receivers needed to be adjusted to roughly 2700 ohms
 * It looked like the analog readings were having issues at comp too. It was either 0-3 ish mph or 41
     * This has been solved on 9/18/24 with dynamically adjusting analog thresholds
 * Analog reads are not ideal for this type of data acquisition. Fast readings are needed, and there should be no margin for error about whether a reading falls within a range
 * Digital readings with a hall sensor, engine spark plug with octocoupler, or some other digital signal could be implemented 


## Electrical Wiring Notes

### Ethernet Cable Wire Colors
* Orange: IR Emitter +
* Orange/White: IR Emitter -
* Blue: IR Receiver +
* Blue/White: IR Receiver -
* Green: TMP36 OUT
* Brown/White: TMP36 3.3V VCC
* Brown: TMP36 GND

