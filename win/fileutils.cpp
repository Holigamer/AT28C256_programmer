#include "fileutils.h"
#include <iostream>

int FileUtils::checkFileForCompatibility(string fileName, int byteSize)
{
    ifstream input(fileName, std::fstream::binary);

    if (!input.is_open())
    {
        // File could not be accessed
        return 1;
    }

    char correctBuf[byteSize + 1];

    int counter = 0;
    // Read as long as not empty
    while (!input.eof())
    {
        // Handle case, if there is more input than the allowed amount
        if (counter > byteSize + 1)
        {
            std::cout << "file too large" << endl;
            return 2;
        }

        input >> correctBuf[counter]; // Read byte into buffer with correct size..
        counter++;
    }

    if (counter != byteSize + 1)
    {
        std::cout << "wrong counter size: " << counter << endl;
        return 2; // File has wrong format.
    }

    input.close();
    return 0;
}

int FileUtils::loadIntoMemory(string fileName, char *buf, int bufSize)
{
    int checkStatus = checkFileForCompatibility(fileName, bufSize);
    if (checkStatus) // If there was an error, return the error
    {
        return checkStatus;
    }

    ifstream input(fileName, std::fstream::binary);

    if (!input.is_open())
    {
        // File could not be accessed
        return 1;
    }
    // Read as long as not empty
    for (int i = 0; i < bufSize; i++)
    {
        input >> buf[i]; // Read byte into buffer with correct size..
    }

    input.close();
    return 0;
}