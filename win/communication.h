#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string>
#include <iostream>
#include <iomanip>
#include "SerialClass.h"

using namespace std;

#define FLASHSIZE 32768

class Communication
{
public:
    /**
     *@brief Prints all the contents of the EEPROM to console. 
     * If displayContent is true, the function displays. buf can be nullptr.. Then it wont be filled..
     */
    static void readContents(Serial *SP, bool displayContent, char buf[32768]);

    /**
     * @brief Writes a complete memory of 32k to arduino
     */
    static void writeFully(Serial *SP, char memory[32768]);

    static void erase(Serial *SP);
};

#endif