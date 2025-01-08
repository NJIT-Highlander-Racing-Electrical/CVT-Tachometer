void checkStatus() {

  /*
    Status flag bits for CVT

    Bit 7 (MSB): Unused
    Bit 6: Unused
    Bit 5: Unused
    Bit 4: Unused
    Bit 3: Secondary Temp
    Bit 2: Secondary IR
    Bit 1: Primary Temp
    Bit 0 (LSB): Primary IR
  */

  // Ternary operator is used for briefness

  // bitWrite(x, n, b)
  // x: the numeric variable to which to write
  // n: which bit of the number to write, starting at 0 for the least-significant (rightmost) bit
  // b: the value to write to the bit (0 or 1)

  bitWrite(statusCVT, 0, ((primaryValue == 0 || primaryValue == 4095) ? 0 : 1)); // If primary IR is at either extreme, we should be concerned
  bitWrite(statusCVT, 1, ((primaryTemperature < 0 || primaryTemperature > 400) ? 0 : 1)); // If temp reads 0 or over 400 degrees, we know something is wrong
  bitWrite(statusCVT, 2, ((secondaryValue == 0 || secondaryValue == 4095) ? 0 : 1)); // If secondary IR is at either extreme, we should be concerned
  bitWrite(statusCVT, 3, ((secondaryTemperature < 0 || secondaryTemperature > 400) ? 0 : 1)); // If temp reads 0 or over 400 degrees, we know something is wrong
}