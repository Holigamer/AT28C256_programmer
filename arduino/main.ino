#include "eeprom.h"
#include "defines.h"

EEPROM eeprom;

void setup()
{

    eeprom = EEPROM();
    Serial.begin(115200);
}

void writeTimedProperly(short addrOnPage, byte data)
{
    eeprom.setAddress(addrOnPage, false);
    // Time between Address valid and WE:  (at least 0 ns)

    // Write Out Data
    for (int i = 0; i < EEPROM::IOBusSize; i++)
    {
        int dparallel = EEPROM::IOBusMSBFirst[i];
        // Write Data to IO Bus one bit at a time, starting with MSB
        digitalWrite(dparallel, data & 0x80);
        data = data << 1;
    }

    digitalWrite(WRITE_ENABLE, LOW);
    // Write Pulse Width: 100ns
    // Each nop: 62.5ns
    __asm__("nop\n\t"
            "nop\n\t"); // 125ns delay
    digitalWrite(WRITE_ENABLE, HIGH);
    __asm__("nop\n\t"
            "nop\n\t"); // 125ns delay
    // Write Pulse Width High: 50ns (min)
    // ByteLoadCycleTime 150µs (max)
}

void loop()
{

    if (Serial.available() > 0)
    {
        uint8_t data = 0x00;
        Serial.readBytes(&data, 1);
        if (data == 't')
        {
            eeprom.outputModeBus();
            for (unsigned int i = 0; i < 64; i++)
            {
                // Write to page
                writeTimedProperly(i, B00001111);
                // Already 125ns delayed. Continue straight up
            }

            // Poll the data..
            eeprom.inputModeBus();
            digitalWrite(WRITE_ENABLE, HIGH);
            char data = 0x00; // Define char to store data into
            unsigned long timeStart =  micros();
            do
            {
                eeprom.setAddress(63, true);
                eeprom.setAddress(63, false);
                eeprom.setAddress(63, true);
                
                for (int i = 0; i < EEPROM::IOBusSize; i++)
                {
                    int dparallel = EEPROM::IOBusMSBFirst[i];
                    // Show DATA
                    data = (data << 1) + digitalRead(dparallel); // Shift data from IO Bus into char
                }
            } while (data==B11110000);

            unsigned long timeNow = micros();

            Serial.print("Time for save action (by polling): ");
            Serial.print(timeNow-timeStart);
            Serial.println("ns");


            // Page write test.
            // Write 64 bytes of data.
            // (Max 150µS between byte)
            // Poll the DATA bus with last byte to check for free memory
        }
        

        if (data == 'p')
        {
            Serial.println("Printing EEPROM");
            eeprom.printContent();
        }
        else if (data == 'e')
        {
            Serial.println("Erase EEPROM with 0xff");
            eeprom.eraseEEPROM();
        }
        else if (data == 'c')
        {
            Serial.print("Erase EEPROM with custom char: 0x");
            // Read the custom clearbyte.
            uint8_t value = 0x00;
            Serial.readBytes(&value, 1);

            Serial.println(value, HEX);
            eeprom.eraseEEPROM(value);
        }
        else if (data == 'r')
        {
            Serial.println("Read value from EEPROM");
            // Read the custom clearbyte.
            uint8_t addressArray[2];
            Serial.readBytes(addressArray, 2);

            uint16_t addr = (addressArray[0] << 8) | addressArray[1];

            byte value = eeprom.readEEPROM(addr);

            Serial.print("Value @ 0x");
            Serial.print(addr, HEX);
            Serial.print(": 0x");
            Serial.println(value, HEX);
        }
        else if (data == 'w')
        {
            // Write to EEPROM
            Serial.println("Write to EEPROM");
            uint8_t addressArray[2];
            uint8_t value[1];
            Serial.readBytes(addressArray, 2);
            Serial.readBytes(value, 1);

            uint16_t addr = (addressArray[0] << 8) | addressArray[1];

            eeprom.outputModeBus();
            eeprom.writeEEPROM(addr, value[0]);

            Serial.print("Written 0x");
            Serial.print(value[0], HEX);
            Serial.print(" to address 0x");
            Serial.println(addr, HEX);
        }

        delay(20);

        Serial.write(data);
    }
}