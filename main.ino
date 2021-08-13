#include "eeprom.h"

EEPROM eeprom;

void setup()
{
    Serial.begin(115200);

    eeprom = EEPROM();
}

void loop()
{
    if (Serial.available() > 0)
    {
        uint8_t data = 0x00;
        Serial.readBytes(&data, 1);

        /**
         * Print all the content of the EEPROM  
         */
        if (data == 'p')
        {
            Serial.println("Printing eeprom");
            eeprom.printContent();
        }

        delay(20);

        Serial.write(data);
    }
}