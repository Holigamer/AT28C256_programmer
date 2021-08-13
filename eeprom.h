#ifndef EEPROM_H
#define EEPROM_H

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
    * Read a specific char of the EEPROM @ adress
    * Part of basic functions
    * Be sure to define IO Bus as Input on all bits.
    */
    char readEEPROM(short address);
    void setAddress(short address, bool output_enable);
};

#endif