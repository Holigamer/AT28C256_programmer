#include "eeprom.h"
#include "defines.h"
#include "helper_methods.h"
#include <HardwareSerial.h>
#include <Arduino.h>

int EEPROM::IOBusMSBFirst[8] = {12, 11, 10, 9, 8, 7, 6, 5};

EEPROM::EEPROM()
{
    // Setup several pins for the EEPROM.
    pinMode(DATASERIAL, OUTPUT);
    pinMode(SHCP, OUTPUT);
    pinMode(STCP, OUTPUT);
    pinMode(WRITE_ENABLE, OUTPUT);

    digitalWrite(SHCP, LOW);
    digitalWrite(WRITE_ENABLE, HIGH);
}

void EEPROM::inputModeBus()
{
    for (int dparallel : IOBusMSBFirst)
    { // Define IO Bus as Input
        pinMode(dparallel, INPUT);
    }
}

void EEPROM::outputModeBus()
{
    for (int dparallel : IOBusMSBFirst)
    { // Define IO Bus as Output
        pinMode(dparallel, OUTPUT);
    }
}

/**
 * Method to print out the entire data storage of the EEPROM
 */
void EEPROM::printContent()
{
    for (int dparallel : IOBusMSBFirst)
    { // Define IO Bus as Input
        pinMode(dparallel, INPUT);
    }

    bool failedVerification = false;

    byte previous_data_cache[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    bool already_skipped_bytes = false;
    for (int i = 0; i <= 3; i++)
    {
        //int i = 0; // Debug only
        for (int base = 0; base <= (8191); base += 16)
        {
            byte data[16];
            for (int offset = 0; offset <= 15; offset += 1)
            {
                data[offset] = readEEPROM((8192 * i) + base + offset);
                /* if (verify0xFF && data[offset] != ((char)0xFF))
                {
                    failedVerification = true;
                }*/
            }

            // Comparing
            if (!array_cmp(data, previous_data_cache, 16, 16))
            {
                already_skipped_bytes = false;
                //Serial.println("debug compared unequal");
                delayMicroseconds(10);
                char buf[80];
                sprintf(buf, "%04x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
                        (8192 * i) + base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
                        data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
                Serial.println(buf);
            }
            else
            {
                // Serial.println("debug compared equal");
                // Arrays are equal
                //Serial.println(",");
                if (!already_skipped_bytes)
                {
                    already_skipped_bytes = true;
                    Serial.println("...");
                }
            }

            memcpy(previous_data_cache, data, sizeof(data[0]) * 16);
        }
    }
    if (already_skipped_bytes)
    {
        char buf[80];
        sprintf(buf, "%04x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
                32752, previous_data_cache[0], previous_data_cache[1], previous_data_cache[2], previous_data_cache[3], previous_data_cache[4], previous_data_cache[5], previous_data_cache[6], previous_data_cache[7],
                previous_data_cache[8], previous_data_cache[9], previous_data_cache[10], previous_data_cache[11], previous_data_cache[12], previous_data_cache[13], previous_data_cache[14], previous_data_cache[15]);
        Serial.println(buf);
    }
    /*if (verify0xFF)
    {
        Serial.print("VERIFY:");
        Serial.println(failedVerification ? "fail" : "success");
    }*/
    Serial.println("end");
}

char EEPROM::readEEPROM(short address)
{
    setAddress(address, /* output Enable */ true); // Latch adress into adress register, output Enable = true
    digitalWrite(WRITE_ENABLE, HIGH);              // Pull write Enable HIGH. Tells EEPROM that its a read OP
    // ORDER: D5=LSB, ..., D12=MSB
    char data = 0x00; // Define char to store data into
    for (int dparallel : IOBusMSBFirst)
    {
        // Show DATA
        data = (data << 1) + digitalRead(dparallel); // Shift data from IO Bus into char
    }
    return data;
}

/**
 * Shift out adress into shift register, than latch it to overwrite current data
 * Part of basic functions
 */
void EEPROM::setAddress(short address, bool output_enable)
{
    // Shift adress to Shift Registers as follows:
    // <WE, MSB, A13, A12, A11, ..., A1, LSB>
    shiftOut(DATASERIAL, SHCP, MSBFIRST, (address >> 8) // Shift first 8 bits into MSB register (BIT Q7 being the OE Pin of EEPROM active LOW)
                                             | (output_enable ? 0x00 : 0x80));
    shiftOut(DATASERIAL, SHCP, MSBFIRST, address); // Shift last 8 bits into LSB register

    digitalWrite(STCP, LOW); // Latch adress into Storage Register
    digitalWrite(STCP, HIGH);
    digitalWrite(STCP, LOW);
}
