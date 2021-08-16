#include "eeprom.h"
#include "defines.h"
#include "helper_methods.h"

//EEPROM eeprom;

void setup()
{

    //eeprom = EEPROM();

    for (int i = DATA_0; i <= DATA_7; i++)
    {
        pinMode(i, OUTPUT);
    }

    // Address latch
    pinMode(LATCH_APPLY, OUTPUT);
    pinMode(LATCH_CLOCK, OUTPUT);
    pinMode(LATCH_DATA, OUTPUT);

    digitalWrite(OUTPUT_ENABLE, LOW);
    digitalWrite(CHIP_ENABLE, HIGH);
    digitalWrite(WRITE_ENABLE, HIGH);

    pinMode(WRITE_ENABLE, OUTPUT);
    pinMode(OUTPUT_ENABLE, OUTPUT);
    pinMode(CHIP_ENABLE, OUTPUT);

    Serial.begin(115200);
}

void setAddr(short addr)
{
    // Shift adress to Shift Registers as follows:
    // <WE, MSB, A13, A12, A11, ..., A1, LSB>
    shiftOut(LATCH_DATA, LATCH_CLOCK, MSBFIRST, (addr >> 8)); // Shift first 8 bits into MSB register

    shiftOut(LATCH_DATA, LATCH_CLOCK, MSBFIRST, addr); // Shift last 8 bits into LSB register
    // Wait for last shift.
    asm("nop\n\t"); // 62.5ns

    digitalWrite(LATCH_APPLY, HIGH); // Latch adress into Storage Register
    asm("nop\n\t");                  // Hold apply (RCLK) for longer
    digitalWrite(LATCH_APPLY, LOW);

    // tpd = c.a. max 50ns @4.5V for 74hc595 (p.8)
    asm("nop\n\t"
        "nop\n\t"); // Generous 125ns delay.

    //OLD
    /* for (int i = ADDR_0; i <= ADDR_14; i++)
    {
        digitalWrite(i, addr & 0x0001);
        addr = addr >> 1;
    }*/
}

void writeTimedProperly(short addrOnPage, byte data)
{

    // Time between Address valid and WE:  (at least 0 ns)
    setAddr(addrOnPage);
    // Write Out Data
    for (int i = DATA_7; i >= DATA_0; i--) // MSB first..
    {
        // Write Data to IO Bus one bit at a time, starting with MSB
        digitalWrite(i, (data & B10000000) != 0); // MSB
        data = data << 1;                         // Shift one to left. 1100 becomes 1000
    }

    digitalWrite(WRITE_ENABLE, LOW);
    // Write Pulse Width: 100ns
    // Each nop: 62.5ns
    asm("nop\n\t"
        "nop\n\t"
        "nop\n\t"); // 187.5ns delay
    //delayMicroseconds(1);
    digitalWrite(WRITE_ENABLE, HIGH);

    asm("nop\n\t"
        "nop\n\t"
        "nop\n\t"); // 187.5ns delay*/
    // Write Pulse Width High: 50ns (min)
    // ByteLoadCycleTime 150µs (max)
}

// Comment in to show polling time of pagewrite.
#define DEBUG_WRITEPAGE
void writePage(short pageStartAddr, byte value)
{
    // Check if page is correct.
    if (pageStartAddr % 64 != 0)
    {
        Serial.println("Wrong startpoint. Must be divisible by 64!");
        return;
    }

    digitalWrite(OUTPUT_ENABLE, HIGH);
    digitalWrite(CHIP_ENABLE, LOW);
    for (int i = DATA_0; i <= DATA_7; i++)
    {
        pinMode(i, OUTPUT);
    }
    for (unsigned int i = pageStartAddr; i < pageStartAddr + 64; i++)
    {
        // Write to page
        writeTimedProperly(i, value);
        // Already 125ns delayed. Continue straight up
    }

    // Poll the data..
    for (int i = DATA_0; i <= DATA_7; i++)
    {
        pinMode(i, INPUT);
    }

    byte content = 0x00; // Define char to store data into
    setAddr(pageStartAddr + 63);
#ifdef DEBUG_WRITEPAGE
    Serial.println("Polling DATA");
    Serial.print("value: ");
    Serial.println(value, HEX);
    unsigned long timeStart = micros();
#endif
    do
    {

        digitalWrite(OUTPUT_ENABLE, LOW);

        content = 0x00;
        for (int i = DATA_7; i >= DATA_0; i--)
        {
            // Write Data to IO Bus one bit at a time, starting with MSB
            content = (content << 1) | digitalRead(i); // Shift data from IO Bus into char
        }

        digitalWrite(OUTPUT_ENABLE, HIGH);
        // Polling has normal Read Waveform Delay => CE is selected, OE is switching, so 70-100ns delay required.
        // Maybe up to 350ns.. depending on chip and which time is valid.. (Datasheet p.11)
        asm("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");       // 500ns delay
                              // WHY?????????????? WHEN REMOVED.. THIS DOESNT WORK!!! :/ OR HANGS UP SOMETIMES..
                              // DEBUGGING DOES NOT DO IT, BECAUSE AS SOON A Serial.println(content, HEX) IS ADDED, THE DELAY IS TO BIG AND THE CODE WORKS!
        delayMicroseconds(1); // 1000ns delay.
#ifdef DEBUG_WRITEPAGE
        Serial.println(content, HEX);
#endif

    } while (content != value);

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

byte readByte(short addr)
{
    digitalWrite(WRITE_ENABLE, HIGH); // Pull write Enable HIGH. Tells EEPROM that its a read OP
    setAddr(addr);                    // Latch adress into adress register, output Enable = true
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

void printContent()
{
    for (int i = DATA_0; i <= DATA_7; i++)
    {
        pinMode(i, INPUT);
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
                data[offset] = readByte((8192 * i) + base + offset);
                /* if (verify0xFF && data[offset] != ((char)0xFF))
                {
                    failedVerification = true;
                }*/
            }

            // Comparing
            if (!array_cmp(data, previous_data_cache, 16, 16))
            {

                //Serial.println("debug compared unequal");
                delayMicroseconds(10);
                char buf[80];
                // END OF OLD SUM
                if (already_skipped_bytes) // Check if this is the start condition.
                {
                    sprintf(buf, "%04x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
                            (8192 * i) + base - 16, previous_data_cache[0], previous_data_cache[1], previous_data_cache[2], previous_data_cache[3], previous_data_cache[4], previous_data_cache[5], previous_data_cache[6], previous_data_cache[7],
                            previous_data_cache[8], previous_data_cache[9], previous_data_cache[10], previous_data_cache[11], previous_data_cache[12], previous_data_cache[13], previous_data_cache[14], previous_data_cache[15]);
                    Serial.println(buf);
                }

                already_skipped_bytes = false;
                // BEGIN OF NEW SUM
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
    Serial.println("e");
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
            for (unsigned int i = 0; i < (unsigned int)32768; i += 64)
            {
                Serial.print("Page: ");
                Serial.println(i);

                unsigned long startTime = micros();
                writePage(i, B11110000); // Debug pattern. 5C
                unsigned long endTime = micros();

                Serial.print("The full page write of EEPROM took: ");
                Serial.print(endTime - startTime);
                Serial.println("µs.");
            }

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
            //shiftOut(LATCH_DATA, LATCH_CLOCK, MSBFIRST, (addr >> 8)); // Shift first 8 bits into MSB register

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
                    // Output low
                    PORTD = PORTD & ~B00000100;
                }
                else
                {
                    // output high
                    PORTD = PORTD | B00000100;
                }

                addr <<= 1;
                // Clock pin is: Bxxxx1xxx
                PORTD = PORTD | B00001000; // Should only take 0.26 µs according to roboticsbackend
                // digitalWrite(LATCH_CLOCK, LOW);
                PORTD = PORTD & ~B00001000; // And with ones complemet of value
            }
            unsigned long timeEnd = micros();

            Serial.print("Custom 4: Address set took: ");
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
            printContent();
        }
        else if (data == 'r')
        {
            for (int i = DATA_0; i <= DATA_7; i++)
            {
                pinMode(i, INPUT);
            }
            Serial.println(readByte(0x00), HEX);
        }

        delay(20);

        Serial.write(data);
    }
}