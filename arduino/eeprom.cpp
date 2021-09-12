#include "eeprom.h"
#include "defines.h"
#include "helper_methods.h"
#include "Arduino.h"

EEPROM::EEPROM()
{
    outputModeBus();

    // Address latch
    pinMode(LATCH_APPLY, OUTPUT);
    pinMode(LATCH_CLOCK, OUTPUT);
    pinMode(LATCH_DATA, OUTPUT);

    pinMode(WRITE_ENABLE, OUTPUT);
    pinMode(OUTPUT_ENABLE, OUTPUT);
    pinMode(CHIP_ENABLE, OUTPUT);

    digitalWrite(OUTPUT_ENABLE, LOW);
    digitalWrite(CHIP_ENABLE, HIGH);
    digitalWrite(WRITE_ENABLE, HIGH);

    
}

void EEPROM::inputModeBus()
{
    for (int i = DATA_0; i <= DATA_7; i++)
    {
        pinMode(i, INPUT);
    }
}

void EEPROM::outputModeBus()
{
    for (int i = DATA_0; i <= DATA_7; i++)
    {
        pinMode(i, OUTPUT);
    }
}

void EEPROM::printContent()
{
    inputModeBus();

    //bool failedVerification = false;

    byte previous_data_cache[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    bool already_skipped_bytes = false;
    bool first16BytesRead = false; // If first line is all zero, it wouldn't get printed.

    for (unsigned int base = 0; base < ((unsigned int)32768); base += 16) // Go through all bytes 16 bytes at a time
    {
        byte data[16];                                  // Cache for one bytearray.
        for (int offset = 0; offset <= 15; offset += 1) // Load data
        {
            data[offset] = readByte(base + offset);
            /* if (verify0xFF && data[offset] != ((char)0xFF))
                {
                    failedVerification = true;
                }*/
        }

        if (array_cmp(data, previous_data_cache, 16, 16) && first16BytesRead) // Are the previous 16 bytes equal to the new buffer?
        {
            if (!already_skipped_bytes) // Have there already been bytes skipped?
            {
                already_skipped_bytes = true;
                Serial.println("..."); // Print these wonderful dots.
            }
            // ==> No? Ok. continue doning nothing.
        }
        else
        {
            first16BytesRead = true; // First line gets printed.
            already_skipped_bytes = false;
            delayMicroseconds(10);
            char buf[80];

            /*if (already_skipped_bytes)
            {
                sprintf(buf, PRINT_CONTENT_FORMAT,
                        (8192 * i) + base - 16, previous_data_cache[0], previous_data_cache[1], previous_data_cache[2], previous_data_cache[3], previous_data_cache[4], previous_data_cache[5], previous_data_cache[6], previous_data_cache[7],
                        previous_data_cache[8], previous_data_cache[9], previous_data_cache[10], previous_data_cache[11], previous_data_cache[12], previous_data_cache[13], previous_data_cache[14], previous_data_cache[15]);
                Serial.println(buf);
            } */

            sprintf(buf, PRINT_CONTENT_FORMAT,
                    base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
                    data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
            Serial.println(buf);
        }

        memcpy(previous_data_cache, data, sizeof(byte) * 16);
    }

    if (already_skipped_bytes) // Print the last line always
    {
        char buf[80];
        //Serial.println("debug printed last");
        sprintf(buf, PRINT_CONTENT_FORMAT,
                32752, previous_data_cache[0], previous_data_cache[1], previous_data_cache[2], previous_data_cache[3], previous_data_cache[4], previous_data_cache[5], previous_data_cache[6], previous_data_cache[7],
                previous_data_cache[8], previous_data_cache[9], previous_data_cache[10], previous_data_cache[11], previous_data_cache[12], previous_data_cache[13], previous_data_cache[14], previous_data_cache[15]);
        Serial.println(buf);
    }
    /*if (verify0xFF)
    {
        Serial.print("VERIFY:");
        Serial.println(failedVerification ? "fail" : "success");
    }*/
    Serial.write('e');
}

byte EEPROM::readByte(unsigned short addr)
{
    digitalWrite(WRITE_ENABLE, HIGH);   // Pull write Enable HIGH. Tells EEPROM that its a read OP
    delayMicroseconds(1);               // Bugfix?
    setAddress(addr);                   // Latch adress into adress register, output Enable = true
    digitalWrite(CHIP_ENABLE, LOW);
    digitalWrite(OUTPUT_ENABLE, LOW);
    delayMicroseconds(1);

    // ORDER: D5=LSB, ..., D12=MSB
    byte data = 0x00; // Define char to store data into
    for (int i = DATA_7; i >= DATA_0; i--)
    {
        // Write Data to IO Bus one bit at a time, starting with MSB
        data = (data << 1) | digitalRead(i); // Shift data from IO Bus into char
    }
    digitalWrite(OUTPUT_ENABLE, HIGH);
    digitalWrite(CHIP_ENABLE, HIGH);
    return data;
}

/*
 * Set addr on EEPROM.
 * Important: The shift out will output any address unil max size of unsigned short (63k) but eeprom has only 15 bits (max 32767) connected.
 */
void EEPROM::setAddress(unsigned short addr)
{
    // Shift adress to Shift Registers as follows:
    // <MSB, A13, A12, A11, ..., A1, LSB>
    uint8_t i;

    for (i = 0; i < 16; i++)
    {
        // One operation takes about 18 µs give or take.
        // Because the first bit is now 16 bits, do a compare with 32768 or 1000 0000 0000 0000 or 0x8000

        // Data PD2     Bxxxxx1xx
        // Clock PD3    Bxxxx1xxx
        // Latch PD4    Bxxx1xxxx
        // digitalWrite(LATCH_DATA, (addr & 0x8000)); // digiWrite takes about 3.40 µs according to roboticsbackend

        if ((addr & 0x8000) == 0)
        {
            PORTD = PORTD & ~B00000100; // LOW
        }
        else
        {
            PORTD = PORTD | B00000100; // HIGH
        }

        addr <<= 1;
        // Clock pin is: Bxxxx1xxx
        PORTD = PORTD | B00001000; // HIGH
        // Pulse duration should be a min of 20ns @4.5V (p.7) This is the case because the arduino only clocks with 62.5ns
        PORTD = PORTD & ~B00001000; // LOW
    }
    // Wait for last shift.
    asm("nop\n\t"); // 62.5ns

    // Latch pin is:   Bxxx1xxxx
    PORTD = PORTD | B00010000;  // HIGH
    asm("nop\n\t");             // Hold apply (RCLK) for longer
    PORTD = PORTD & ~B00010000; // LOW

    // tpd = c.a. max 50ns @4.5V for 74hc595 (p.8)
    asm("nop\n\t"
        "nop\n\t"); // Generous 125ns delay.
}

void EEPROM::writeByteTimedProperly(unsigned short addrOnPage, byte data)
{

    // Time between Address valid and WE:  (at least 0 ns)
    setAddress(addrOnPage);
    // Write Out Data
    for (int i = DATA_7; i >= DATA_0; i--) // MSB first..
    {
        // Write Data to IO Bus one bit at a time, starting with MSB
        // @For now not optimized, because working with two diffrent ports.
        digitalWrite(i, (data & B10000000) != 0); // MSB
        data = data << 1;                         // Shift one to left. 1100 becomes 1000
    }

    // WRITE_ENABLE @ PB5: Bxx1xxxxx 
    PORTB = PORTB & ~B00100000; // LOW

    // Write Pulse Width: min 100ns
    asm("nop\n\t"
        "nop\n\t"
        "nop\n\t"); // 187.5ns delay

    PORTB = PORTB | B00100000; // HIGH

    asm("nop\n\t"
        "nop\n\t"
        "nop\n\t"); // 187.5ns delay
    // Write Pulse Width High: 50ns (min)
    // ByteLoadCycleTime 150µs (max)
}


void EEPROM::writePage(unsigned short pageStartAddr, byte value[64])
{
    // Check if page is correct.
    if (pageStartAddr % 64 != 0)
    {
        Serial.println("Wrong startpoint. Must be divisible by 64!");
        return;
    }

    digitalWrite(OUTPUT_ENABLE, HIGH);
    digitalWrite(CHIP_ENABLE, LOW);
    outputModeBus();
    for (unsigned int i = pageStartAddr; i < pageStartAddr + 64; i++)
    {
        // Write to page
        writeByteTimedProperly(i, value[i - pageStartAddr]);
        // Already 125ns delayed. Continue straight up
    }

    inputModeBus();

    byte content = 0x00; // Define char to store data into
    setAddress(pageStartAddr + 63);
#ifdef DEBUG_WRITEPAGE
    Serial.println("Polling DATA");
    Serial.print("value: ");
    Serial.println(value, HEX);
    unsigned long timeStart = micros();
#endif
    do
    {

        // This does fail sometimes (hang up?).. Idk why.
        // Dirty solution: When flashing, mesure the responses of arduino as some sort of heartbeat. IOf longer than f.ex. 1s, reset the entire flash.
        digitalWrite(OUTPUT_ENABLE, LOW);
        delayMicroseconds(1);
        content = 0x00;
        for (int i = DATA_7; i >= DATA_0; i--)
        {
            // Write Data to IO Bus one bit at a time, starting with MSB
            content = (content << 1) | digitalRead(i); // Shift data from IO Bus into char
        }

        digitalWrite(OUTPUT_ENABLE, HIGH);
        // Polling has normal Read Waveform Delay => CE is selected, OE is switching, so 70-100ns delay required.
        // Maybe up to 350ns.. depending on chip and which time is valid.. (Datasheet p.11)
        delayMicroseconds(2); // 2000ns delay.
#ifdef DEBUG_WRITEPAGE
        //Serial.println(content, HEX);
#endif

    } while (content != value[63]); // Always check last value.

#ifdef DEBUG_WRITEPAGE
    unsigned long timeNow = micros();
#endif

    digitalWrite(CHIP_ENABLE, HIGH);

#ifdef DEBUG_WRITEPAGE

    Serial.print("Stored Byte: ");
    Serial.println(content, HEX);

    Serial.print("Time for save at ");
    Serial.print(pageStartAddr, HEX);
    Serial.print(" (by polling): ");
    Serial.print(timeNow - timeStart);
    Serial.println("µs");
#endif
}

void EEPROM::eraseEEPROM(const byte data)
{

    byte pageContent[64];
    for (int i = 0; i < 64; i++)
    {
        pageContent[i] = data;
    }
    for (unsigned int i = 0; i <= (unsigned int)32767; i += 64)
    {
        #ifdef DEBUG_ERASE
        Serial.print("Page: ");
        

        unsigned long startTime = micros();
        #endif
        char buf[5];
        sprintf(buf, "p%04x", i);
        Serial.write(buf); // output page of erased page in ASCII (formatted to hex)
        writePage(i, pageContent); 
        #ifdef DEBUG_ERASE
        unsigned long endTime = micros();

        Serial.print(" erase took: ");
        Serial.print(endTime - startTime);
        Serial.println("µs.");
        #endif
    }
    Serial.write('e');
}
