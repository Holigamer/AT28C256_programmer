#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>

#define STRICT_SILENT_MODE // Comment out if in debug.
#define PRINT_CONTENT_FORMAT "l%04x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x"

// Comment in to show polling time of pagewrite.
//#define DEBUG_WRITEPAGE

//#define DEBUG_ERASE
/**
 * @class EEPROM - This class provides IO functionality to interface with the AT28C256 EEPROM.
 * 
 */
class EEPROM
{

public:
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
    byte readByte(unsigned short address);

    /**
     * Shift out adress into shift register, than latch it to overwrite current data
     * Part of basic functions
     */
    void setAddress(unsigned short address);

    /**
     * Write a page of data. (currently broken.)
     */
    void writePage(unsigned short address, byte data[64]);

    /**
     * As eraseEEPROM(byte), but with 0xff as default.
     */
   // void eraseEEPROM();

    /**
     * Erase contents of eeprom and write data into every place.
     */
    void eraseEEPROM(const byte data);

private:
    /**
    * Write a byte to the EEPROM as fast as allowed, but without any chipselect lines. Only for internal use.
    */
    void writeByteTimedProperly(unsigned short addrOnPage, byte data);
};

#endif