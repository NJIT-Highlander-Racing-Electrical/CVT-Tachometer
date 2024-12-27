 # CVT-Tachometer

A subsystem that measures the RPM of the primary and secondary on the CVT. This subsystem also has a temperature sensor to detect if the CVT is overheating.


## 2024-2025 Design Goals

* Everything last year seemed to work well, but a few improvements to the existing system could be made:
  * Software
       * Improve program to do more analog reads and spend less time processing data
          * This may involve constants that are only calculated once during setup, faster math functions, fewer delays, etc
       * Use the analog reads for IR sensors on both ADC1 and ADC2 to prevent any issues. 
  * Sensor Enclosures
       * the sensor housings should be made out of something other than PLA
       * they should be able to fit custom PCBs with connectors
       * the hood around the emitter and receiver should be enlarged to prevent anything else from reflecting other than what is directly in front
  * PCBs for sensors
       * the wiring from the sensors to the main electronics housing were tough to do
       * A small PCB for the electronics would be nicer than hand soldering wires
       * A connector would also be good so that they can be removed/swapped easily
            * Maybe 1x4 or 1x5 or 1x6 JST depending on how many conductors we need going to each board
  * Additional TMP36 Sensor
      * Another TMP36 temperature sensor can be added so that there is only one version of the sensor PCB, and we get a better idea of the temperature gradient within the CVT case (primary vs secondary).
  * Additional Note
     * On the IR receiver output, have it split to two GPIO. One leg has a potentiometer (0-10k?) between the output and the GPIO for finding the optimal resistance. Also make sure it has a small series resistor to prevent resistance from dropping completely to zero. The other GPIO has a fixed resistor that can be added after the fact that will be used in the long term. 

## Hardware Components:

* IR Sensors that point at primary and secondary.
- These can detect when a white stripe on the CVT passes by (one revolution)
* Microcontroller to convert the number of revolutions per second into an RPM value for dashboard and DAS (via CAN-Bus)
  - Uses ESP32 and TJA1051T CAN-Bus transceiver
* TMP36 Temperature Sensor to look at ambient temperature in the CVT case
- Sends a bit via CAN-Bus to turn on an overheating status LED on the dashboard

 ## Notes

 * Super-Bright IR LEDs support an output current of 100mA with a forward voltage of 1.6V. We are using a 22 ohm resistor for a current of roughly 80 mA
    * 5V -> IR LED -> Resistor -> Ground
 * Using standard infrared photodiodes, we can simply power the IR leds without needing a 38kHz pulse rate
    * 5V -> IR Diode -> Resistor -> Ground (sample signal between diode and resistor voltage divider)
 * Temperature sensor is TMP36
    * 5V - > TMP36 VCC, GND -> TMP 36 GND, VOUT -> GPIO
 * Connections from the LEDs/sensors to the main body should be sealed/waterproof
 * The holes in the CVT case should be 20mm
 * The infrared diode appears to act like a variable resistor, but voltage polarity must be + on cathode (short negative leg) and - on anode (short positive leg)

 * The software utilizes the dual core functionality of the ESP32
   * Core 1 reads primaryRPM and deals with CAN-Bus communication
      * It is very important that all other processing is kept to a minimum to ensure primaryRPM is accurately read
   * Core 0 solely reads secondaryRPM

### Previous Issues 
 * One issue noticed during the testing phase was that the analog sensor readings were constantly peaking (4095). The resistor divider for the IR receivers needed to be adjusted to roughly 2700 ohms
 * It looked like the analog readings were having issues at comp too. It was either 0-3 ish mph or 41
     * This has been solved on 9/18/24 with dynamically adjusting analog thresholds
 * Analog reads are not ideal for this type of data acquisition. Fast readings are needed, and there should be no margin for error about whether a reading falls within a range
 * Digital readings with a hall sensor, engine spark plug with octocoupler, or some other digital signal could be implemented 


## Electrical Wiring Notes

### Pins to sensor modules

* Pin 1: +5V
* Pin 2: GND
* Pin 3: TMP36 Output
* Pin 4: IR Emitter Return Line
* Pin 5: IR Receiver Return Line

