#include <iostream>
#include <fstream>
#include <tchar.h>
#include <string>
#include "SerialClass.h" // Library described above
#include "communication.h"
#include "fileutils.h"
using namespace std;

// application reads from the specified serial port and reports the collected data
int main(int argc, char const *argv[])
{
	if (argv[1] == nullptr || argv[2] == nullptr)
	{
		cout << "Please specify comPort and filePath: ex.:'flash.exe COM6 user\\rom.bin'" << endl;
		return 0;
	}
	cout << "Connecting to Port: '" << argv[1] << "'" << endl
		 << endl;

	string file(argv[2]);
	string comPort(argv[1]);

	char memory[FLASHSIZE];

	if (int i = FileUtils::loadIntoMemory(file, memory, FLASHSIZE) != 0)
	{
		// There has been an error while loading file..
		switch (i)
		{
		case 1:
			// Error with file
			cout << "Could not open file '" << file << "'. Please double check, if program has permission and file exists." << endl;
			break;
		case 2:
			cout << "The Binary size of the file does not match '" << FLASHSIZE << "'! Please use a correct binary image." << endl;
			break;
		default:
			cout << "Unknown error while opening file..." << endl;
			break;
		}
		return 0;
	}

	Serial *SP = new Serial(comPort.data()); // adjust on what serial port the flash device lies as needed

	if (SP->IsConnected())
		cout << "Connection to Arduino was successful." << endl;
	else
	{
		cout << "Could not connect to Arduino on Port: '" + comPort + "'! Please double check if it is the correct Port." << endl;
		return 0;
	}
	cout << "-------------------" << endl;
	//TODO: Problem with multiple functions after one another
	//Erase first..
	cout << "Erasing EEPROM..." << endl;
	Communication::erase(SP);
	Sleep(100);
	cout << "-------------------" << endl
		 << "Flashing new image.." << endl;
	Communication::writeFully(SP, memory);
	Sleep(100);
	cout << "-------------------" << endl
		 << "Checking for errors in EEPROM.." << endl;
	char buf[FLASHSIZE];
	Communication::readContents(SP, false, buf);
	cout << "Correction check for buffer:" << endl;

	bool error = false;

	for (int i = 0; i < FLASHSIZE; i++)
	{
		if (buf[i] != memory[i])
		{
			// The flashed bit does not match the current..
			// Correct for that..
			//cout << "Byte error @addr " << i << "! Should be '" << memory[i] << "'. Is: '" << buf[i] << "'! PLEASE IMPLEMENT CORRECTION" << endl;
			error = true;
		}
		if (i % 1024 == 0)
		{
			cout << "." << flush;
		}
	}
	cout << endl;
	if (!error)
	{
		cout << "All bytes are ok!" << endl;
	}
	else
	{
		cout << "ERROR WITH SOME BYTES!" << endl;
	}
	cout << "-------------------" << endl;

	Sleep(1);

	return 0;
}