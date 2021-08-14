#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include "SerialClass.h" // Library described above
#include <string>

using namespace std;

// application reads from the specified serial port and reports the collected data
int main(int argc, char const *argv[])
{
	cout << "Connecting to Port: '" << argv[1] << "'" << endl
		 << endl;

	Serial *SP = new Serial(argv[1]); // adjust as needed

	if (SP->IsConnected())
		printf("Connection successful.");

	char incomingData[256] = ""; // don't forget to pre-allocate memory
	//printf("%s\n",incomingData);
	int dataLength = 255;
	int readResult = 0;

	char cmd[16]={'p', (char)0x00, (char)0x02, (char)0x00};//{'p', (char)0x00};////{'w', (char)0x7f, (char)0xff, (char) 0xaa};

	if (true)
	{

		cout << "Sending following char: '" << cmd << "'" << endl;

		if (!SP->WriteData(cmd, 4))
		{
			cout << "Error while sending data to Arduino." << endl;
		}
	}

	while (SP->IsConnected())
	{
		readResult = SP->ReadData(incomingData, dataLength);
		//printf("Bytes read: (0 means no data available) %i\n", readResult);

		incomingData[readResult] = 0;
		printf("%s", incomingData);

		//std::cout << "Halting." << std::endl;

	}

	Sleep(1);

	return 0;
}