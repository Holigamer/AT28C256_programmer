#ifndef DEFINES_H
#define DEFINES_H

#define DATA_0 5
#define DATA_7 12
// Address space:
#define LATCH_DATA 2            // Data Output to Address shift register
#define LATCH_CLOCK 3          //  clock pulse for shift register
#define LATCH_APPLY 4          //  apply or latch into storage register (output to eeprom)

#define WRITE_ENABLE 13
#define OUTPUT_ENABLE A0
#define CHIP_ENABLE A1

#endif