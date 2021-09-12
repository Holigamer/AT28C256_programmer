#include "communication.h"

void Communication::readContents(Serial *SP, bool displayContent, char memory[32768])
{
    if (!SP->IsConnected())
    {
        cout << "Could not create connection over serial." << endl;
        return;
    }

    // Print content of EEPROM (p) [see doc]
    if (!SP->WriteData("p", 1)) // Sends a 'p' to the arduino
    {
        cout << "Error while sending data to Arduino." << endl;
        return;
    }

    char data[64] = ""; // don't forget to pre-allocate memory
    char dataBuf;

    int dataPointer = 0; // Where in the data array the current char should be stored.
    int readResult = 0;  // How many bytes have been read (0 if there was no new message)

    unsigned int lastSuccessfullAddress = 0;

    while (SP->IsConnected())
    {
        readResult = SP->ReadData(&dataBuf, 1);
        if (readResult != 0)
        {
            data[dataPointer] = dataBuf; // Buffered data into array
            dataPointer++;
            if (dataPointer > 63)
            { // Rollover because the data buffer is only 64 chars long.
                dataPointer = 0;
            }
            if (dataBuf == '\n') // Read line has been aquired.
            {
                // Handle the buffer and return to zero
                dataPointer = 0;
                switch (data[0])
                {
                case 'l': // New data coming in. Format: l<4 char hex addr>,( <2 char hex data>, )*16
                {
                    string addrAndData = string(data);

                    unsigned int addr = std::stoi(addrAndData.substr(1, 4), nullptr, 16); // Convert from base 16 number to int.
                    if (displayContent)
                        printf("%04x: ", addr & 0xFFFF); // Masked with 0xFFFF to only give the last four positions. Else there would be 32 bytes in output (Yea yea.. this is hacky and will be fixed in the future)

                    // Check if there has been a skip in data...
                    if ((addr - 16) != lastSuccessfullAddress)
                    {
                        // Fill with lastSuccessfull content...
                        unsigned int addrDiff = addr - lastSuccessfullAddress; // This number will always be positive, because the printout is incremental.
                        for (int lines = 0; lines < addrDiff / 16; lines++)    // How many lines (a 16 bytes) have been skipped..
                        {
                            for (int i = 0; i < 16; i++) // fill a single line with previous content..
                            {
                                /**
                                 * Example for some memory getting filled.
                                 * Previous state:
                                 * 0000: 1,2,3,4    >lastSuccessfullAddress
                                 * 0016: ...
                                 * 0032: ...
                                 * 0048: current    >addr
                                 * 
                                 * addrDiff = 32
                                 * lines = {0,1}
                                 * 
                                 * newIndexes = (0000+16+0+0, 0000+16+16*1+15)
                                 * Current State:
                                 * 0000: 1,2,3,4
                                 * 0016: 1,2,3,4
                                 * 0032: 1,2,3,4
                                 * 0048: current
                                 */
                                if (memory != nullptr)
                                    memory[lastSuccessfullAddress + 16 + 16 * lines + i] = memory[lastSuccessfullAddress + i];
                            }
                        }
                    }
                    // Now read in the 16 times 2 bytes of data..
                    char mempage16[16]; // Memorypage of 16 bytes
                    for (int i = 0; i < 16; i++)
                    {
                        int substrOffset = 6 + (i * 3);
                        mempage16[i] = (char)std::stoi(addrAndData.substr(substrOffset, 2), nullptr, 16);

                        // Fill memory also:
                        if (memory != nullptr)
                            memory[addr + i] = mempage16[i];

                        if (displayContent)
                            printf("%02x ", mempage16[i] & 0xFF); // Print out formatted in hex.
                        if (i == 7)
                            if (displayContent)
                                cout << "  ";
                    }

                    lastSuccessfullAddress = addr; // Save addr if some lines are skipped...
                                                   // Output ends here. The mempage is now fully loaded with 16 elements.
                    if (displayContent)
                        cout << endl;
                }
                break;
                case '.':
                    // The previous data shall be carried over the next 16 bytes
                    if (displayContent)
                        cout << "..." << endl;
                    break;
                case 'e':
                    if (displayContent)
                        cout << "End of print operation" << endl;
                    return;
                default:
                    cout << "Error in communication. data0: " << data[0] << endl;
                    return;
                }
            }
        }
    }
}

void Communication::writeFully(Serial *SP, char memory[32768])
{
    if (!SP->IsConnected())
    {
        cout << "Could not create connection over serial." << endl;
        return;
    }

    cout << std::setprecision(3);

    char incomingData; // don't forget to pre-allocate memory
    //printf("%s\n",incomingData);
    int readResult = 0;

    if (!SP->WriteData("f", 1)) // Flash command
    {
        cout << "Error while sending data to Arduino." << endl;
    }

    int position = 0;
    int flashPosition = 0;
    bool printEndOfExchange = false;
    unsigned int returnedFlashSize = 0; // What the Arduino confirms is the actually flashed amount of data.

    while (SP->IsConnected())
    {
        readResult = SP->ReadData(&incomingData, 1);
        if (readResult != 0)
        {
            if (printEndOfExchange)
            {
                if (position == -2)
                {
                    // HIb
                    returnedFlashSize = ((unsigned int)incomingData) << 8;
                    position++;
                }
                else
                {
                    returnedFlashSize |= ((unsigned int)incomingData);

                    cout << "Flash bytes written to memory: " << returnedFlashSize << endl
                         << "End of write" << endl;
                    return;
                }
            }
            else
            {
                switch (incomingData)
                {
                case 'b': // Send flash size to EEPROM
                { // Position = 0
                    if (position < 1)
                    {
                        cout << "Sending bytesize of " << FLASHSIZE << endl;
                        unsigned int flashSize = FLASHSIZE;
                        char buf[2] = {(flashSize >> 8) & 0b11111111, flashSize & 0b11111111};
                        SP->WriteData(buf, 2);
                        position++; // Increase to 1
                    }
                    else
                    {
                        cout << "Wrong communication. @ bytesize" << endl;
                        return;
                    }
                }
                break;
                case 'r':
                {
                    position++;
                    for (int i = 0; i < 64; i++) // Send one byte 64 times
                    {
                        if (!SP->WriteData(&memory[flashPosition + i], 1)) // Pointer to memory array @ index flashPosition+current byte = (0-32767)
                        {
                            cout << "error with bytewrite: byte:" << memory[flashPosition + i] << " index:" << flashPosition + i << endl;
                            return;
                        }
                    }
                    double percentDone = ((double)flashPosition) / ((double)FLASHSIZE) * 100;
                    cout << percentDone << "% " << flush;
                    if (flashPosition % 1024 == 0 && flashPosition != 0) // Output an endline to format code a little bit better :)
                        cout << endl;
                    //printf("Write page @%04x\n", flashPosition);
                    flashPosition += 64;
                }
                break;
                case 'e':
                {
                    printEndOfExchange = true;
                    position = -2;
                }
                break;
                default:
                    cout << "Error in communication protocol. Position: " << position << " char: " << incomingData << endl;
                    return;
                }
            }
        }
    }
}

void Communication::erase(Serial *SP)
{
    if (!SP->IsConnected())
    {
        cout << "Could not create connection over serial." << endl;
        return;
    }

    cout << std::setprecision(3);

    char incomingData; // don't forget to pre-allocate memory
    int readResult = 0;
    int pageCounter = 0;
    char pageCount[5];

    if (!SP->WriteData("e", 1)) // Erase command.
    {
        cout << "Error while sending data to Arduino." << endl;
    }
    while (SP->IsConnected())
    {
        readResult = SP->ReadData(&incomingData, 1);
        if (readResult != 0)
        {
            if (pageCounter != 0) // Currently reading address of cleared memory..
            {
                pageCount[pageCounter - 1] = incomingData;
                pageCounter++;
                if (pageCounter >= 5) // 4 means last bit was set and correct for pageCounter increase
                {
                    pageCounter = 0;                                                      // Reset this bch.
                    int currentAddress = std::stoul(std::string(pageCount), nullptr, 16); // Can cast to signed int, because address will not reach 31 bit max.
                    double percentDone = ((double)currentAddress) / ((double)FLASHSIZE) * 100.0;
                    cout << percentDone << "% " << flush;
                    if (currentAddress % 1024 == 0 && currentAddress != 0) // Output an endline to format code a little bit better :)
                        cout << endl;
                }
                // Continue reading..
            }
            else if (incomingData == 'e')
            {
                cout << endl
                     << "End of erase." << endl;
                return;
            }
            else if (incomingData == 'p')
            {
                pageCounter = 1;
            }
            else
            {
                cout << "There has been an error with communication to EEPROM.. byte: " << incomingData << " position: " << pageCounter << endl;
                return;
            }
        }
    }
    return;
}