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
        if (data == 'e')
        {
            Serial.println("Writing content.");
            eeprom.eraseEEPROM(0xff);
        }
        else if (data == 'f')
        {
        // Flash communication with arduino.
        // Do some initialization procedures in future here.
        wrong_bytesize:                       // When flashSize == 0 || not multiple of 64
            Serial.write('b');                // Ask for size of flash.
            while (!(Serial.available() > 1)) // Buffer two bytes
            {
                // No new two byte value received, wait.
                delay(100); // 100ms delay.
            }

            uint8_t flashSizeBuffer[2] = {0, 0};
            Serial.readBytes(flashSizeBuffer, 2); // Two bytes.

            unsigned int flashSize = (((unsigned int)flashSizeBuffer[0]) << 8) | flashSizeBuffer[1]; // MSB | LSB

            if (flashSize <= 0 || flashSize % 64 != 0)
            {
                // Cannot write zero bytes on the flash.
                goto wrong_bytesize;
            }

            for (unsigned short i = 0; i < flashSize; i += 64)
            {
                // Pagewrite block
                Serial.write('r');
                uint8_t pageBuffer[64];
                byte pos = 0;
                // Buffer size is not 64 bytes :(

                while (1)
                {
                    if (pos > 63) // Stop when 64 bytes filled (address 63)
                        break;
                    if (Serial.available() > 0)
                    {
                        pageBuffer[pos] = Serial.read();
                        pos++;
                    }
                }

                // Write pagebuffer to EEPROM.
                eeprom.writePage(i, pageBuffer);
            }

            // Done with pagewrites.
            Serial.write('e'); // End of negotiated flashSize
            Serial.write(flashSizeBuffer[0]);
            Serial.write(flashSizeBuffer[1]);
        }
        else if (data == 'p')
        {
            eeprom.printContent();
        }

        delay(20);

        Serial.write(data);
    }
}