#include "eeprom.h"
#include "defines.h"
#include "helper_methods.h"

EEPROM eeprom;

void setup()
{
    eeprom = EEPROM();
    Serial.begin(115200);
}

void loop()
{

    if (Serial.available() > 0)
    {
        uint8_t data = 0x00;
        Serial.readBytes(&data, 1);
        if (data == 't')
        {
            Serial.println("Writing content.");
            eeprom.eraseEEPROM(0xfa);

            // Page write test.
            // Write 64 bytes of data.
            // (Max 150µS between byte)
            // Poll the DATA bus with last byte to check for free memory
        }
        else if (data == 'w')
        {
            // Code to test writeTimedProperly
            Serial.println("test writeTimedProperly");

            unsigned short addr = 0x0040;
            digitalWrite(OUTPUT_ENABLE, HIGH);
            digitalWrite(CHIP_ENABLE, LOW);
            for (int i = DATA_0; i <= DATA_7; i++)
            {
                pinMode(i, OUTPUT);
            }

            unsigned long timeStart = micros();

            // Put the code to time in here..

           // for (int i = 0; i < 1000; i++)
                //writeTimedProperly(addr, B00001111);
            unsigned long timeEnd = micros();

            Serial.print("Custom 6: writeTimedProperly x1000 took: ");
            Serial.print(timeEnd - timeStart);
            Serial.println("µs.");

            for (int i = DATA_0; i <= DATA_7; i++)
            {
                pinMode(i, INPUT);
            }

            // Serial.print("value: ");
            // Serial.println(readByte(addr), HEX);
        }
        else if (data == 'p')
        {
            Serial.println("Printing content of EEPROM");
            eeprom.printContent();
        }
        else if (data == 'r')
        {
            for (int i = DATA_0; i <= DATA_7; i++)
            {
                pinMode(i, INPUT);
            }
            Serial.println(eeprom.readByte(0x00), HEX);
        }

        delay(20);

        Serial.write(data);
    }
}