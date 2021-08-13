#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>

/**
 * @class EEPROM - This class provides IO functionality to interface with the AT28C256 EEPROM.
 * 
 */
class EEPROM
{

public:
    static int IOBusMSBFirst[8];

    static void outputModeBus();
    static void inputModeBus();

    EEPROM();

    /**
    * Output content of the EEPROM to Serial Console. Already formatted. 
    */
    void printContent();

    /**
    * Read a specific char of the EEPROM @ address
    * Part of basic functions
    * Be sure to define IO Bus as Input on all bits.
    */
    char readEEPROM(short address);

    /**
     * Shift out adress into shift register, than latch it to overwrite current data
     * Part of basic functions
     */
    void setAddress(short address, bool output_enable);

    /**
     * Write a specific byte to the EEPROM @ address
     * Part of basic functions
     * Be sure to define IO Bus as Output on all bits.
     * 
     * (for timings see chapter 15. AC Write Waveforms of AT28C256-15PU datasheet)
     * 
     * @see outputModeBus();
     */
    void writeEEPROM(short address, byte data);

    /**
     * As eraseEEPROM(byte), but with 0xff as default.
     */
    void eraseEEPROM();

    /**
     * Erase contents of eeprom and write data into every place.
     */ 
    void eraseEEPROM(byte data);
};

#endif