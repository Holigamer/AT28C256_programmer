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
            //Serial.println("Writing content.");
            eeprom.eraseEEPROM(0xff); // Clear the EEPROM With 0xff
        }
        else if (data == 'c')
        {
            Serial.print('r');                // Ready for the byte, that should be flashed with..
            while (!(Serial.available() > 0)) // Buffer one bytes
            {
                // No new one byte value received, wait.
                delay(100); // 100ms delay.
            }

            uint8_t eraseByte = Serial.read();

            eeprom.eraseEEPROM(eraseByte); // Clear the EEPROM with custom value.
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
        else if (data == 'r')
        {
            // Convert to HEX
            char buf[40];
            String s = Serial.readString();
            Serial.println(s);
            s.toCharArray(buf,40);
            unsigned short hexAdress = strtoul(buf, 0, 16);

            Serial.print("Showing Contents of EEPROM @ 0x");
            Serial.print(hexAdress, HEX);
            Serial.print(": ");

            eeprom.inputModeBus();
            Serial.println(eeprom.readByte(hexAdress), HEX);
            Serial.println("e");
        }
       /*  else if (data=='w')
        {   // Write a single Byte to a given adress.
            // Getting the hex out of #2
            char buf[40];
            Serial.readBytes(buf, 40);
            byte hexDATA = strtoul(buf, NULL, 0);                // Converted Byte contained in String to Byte contained in Adress.

            getValue(receivedChar, ' ', 1).toCharArray(buf, 40); // byte to be written (e.x. FF or 5F)
            short hexAdress = strtoul(buf, NULL, 0);
            // Print Message to Console.
            Serial.print("Writing 0x");
            Serial.print(hexDATA, HEX);
            Serial.print(" to EEPROM @ 0x");
            Serial.println(hexAdress, HEX);

            for (int dparallel : IOBusMSBFirst)
            { // Define IO Bus as Output
                pinMode(dparallel, OUTPUT);
            }

            // Flash data onto EEPROM
            write_eeprom(hexAdress, hexDATA);
            Serial.println("end");
        } */

        delay(20);

        //Serial.write(data);
    }
}