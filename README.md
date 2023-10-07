 # CVT-Tachometer

A subsystem that measures the RPM of the primary and secondary on the CVT. This subsystem also has a temperature sensor to detect if the CVT is overheating.

## Potential Hardware Solutions:
* IR Sensors that point at primary and secondary.
- These can detect when a white stripe on the CVT passes by (one revolution)
* Microcontroller to convert the number of revolutions per second into an RPM value for dashboard and DAS (via CAN-Bus)
  - Using SEED STUDIO XIAO SAMD21:
  - IR Receivers on D1 and D2 as rising edge interrupt
  - D3 for TMP36
  - D8, D9, D10 as SCK, MISO, MOSI and D4 as CS?
  - IR Blasters permanently tied to 5V
* Temperature Sensor (TMP36?) to look at ambient temperature in the CVT case
- Sends a bit via CAN-Bus to turn on an overheating status LED on the dashboard
