#include <deque>
#include "receive.h"
#include "protocol.h"
#include "crc.h"

using namespace std;
bool receivingMode = false;
COMMTIMEOUTS timeouts = { 0, 0, 0, 0, 0 };

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	ReadChar
--
-- DATE: 		November 19, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Sebastian Pelka
--
-- PROGRAMMER: 	Sebastian Pelka
--
-- INTERFACE: 	char ReadChar(DWORD timeout)
--					DWORD timeout	: the timeout to be applied to the ReadFile method within this function
--
-- RETURNS: 	the char read in from the input buffer.
--
-- NOTES:
-- Reads one character from the input buffer and returns it.
----------------------------------------------------------------------------------------------------------------------*/
char ReadChar(DWORD timeout)
{
	timeouts.ReadIntervalTimeout = timeout;
	char received = NUL;
	// set timeouts
	if (!SetCommTimeouts(GetWConn().hComm, &timeouts))
	{
		// Error setting time-outs.
	}

	//read in character from comm port
	//if you fail to read in a file, return NUL
	ReadFile(GetWConn().hComm, &received, 1, NULL, NULL);
	
	return received;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	FillRXBuffer
--
-- DATE: 		November 19, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Sebastian Pelka
--
-- PROGRAMMER: 	Sebastian Pelka
--				Georgi Hristov
--
-- INTERFACE: 	bool FillRxBuffer()
--
-- RETURNS: 	true if the function succeeds in reading all data from the port
--
-- NOTES:
-- When a character is received on the input buffer, an event is generated, and the contents of the input buffer are
-- read in and copied to the GetWConn recieve buffer.
--
-- SOURCE: http://msdn.microsoft.com/en-us/library/ff802693.aspx
----------------------------------------------------------------------------------------------------------------------*/
bool FillRxBuffer()
{
	char controlChar;
	char buffer[PACKET_SIZE];
	DWORD dwCommEvent;
	DWORD dwRead = 0;

	//if the comm mask is successfully set to watch for receiving character events
	if (!SetCommMask(GetWConn().hComm, EV_RXCHAR))
	{
		return false;
	}

	for (;;)
	{
		if (WaitCommEvent(GetWConn().hComm, &dwCommEvent, NULL))
		{
			do
			{
				if (ReadFile(GetWConn().hComm, &controlChar, 1, NULL, NULL))
				{
					//if the data in the buffer is a packet
					if (controlChar == EOT || controlChar == ETB)
					{
						//if you successfully read the packet in
						if (ReadFile(GetWConn().hComm, &dwRead, PACKET_SIZE - 1, NULL, NULL))
						{
							//push the characters into the receive buffer
							for (unsigned int i = 1; i < PACKET_SIZE; i++)
							{
								(GetWConn().buffer_receive).push_back(buffer[i]);
							}
						}
					}
					
				}
				else
				{
					//an error occured while reading in the file
					break;
				}
			} while (dwRead);
		}
		else
		{
			//error in WaitCommEvent
			break;
		}
	}
	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	CheckForETX
--
-- DATE: 		November 29, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Sebastian Pelka
--
-- PROGRAMMER: 	Sebastian Pelka
--
-- INTERFACE: 	bool CheckForETX()
--
-- RETURNS: 	true if the ETX character is found
--
-- NOTES:
-- Iterates over the WConn received buffer, if the CRC validator function confirmed the received data is good. This
-- function pulls the data out of the packet structure and adds it to the print buffer, which containes all the
-- validated data to be printed to the screen so far.
----------------------------------------------------------------------------------------------------------------------*/
bool CheckForETX()
{
	bool ETXfound = false;
	deque<char> packet; // some dummy variables to use while we figure out where to put the global
	deque<char>::iterator packetIterator = GetPrintBuffer().received.begin();
	deque<char> temp;

	for (unsigned int i = 0; i < 1018; i++)
	{
		if ((*packetIterator) == ETX)
		{
			ETXfound = true;
			break;
		}
		else
		{
			temp.push_back(*packetIterator);
		}
		packetIterator++;
	}
	return ETXfound;

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	ClearPrintBuffer()
--
-- DATE: 		November 29, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Sebastian Pelka
--
-- PROGRAMMER: 	Sebastian Pelka
--
-- INTERFACE: 	void ClearPrintBuffer()
--
-- NOTES:
-- Clears the received print buffer. This function is intended to be called after the contents of the print buffer
-- are painted to the screen.
----------------------------------------------------------------------------------------------------------------------*/
void ClearPrintBuffer()
{
	if (!GetPrintBuffer().received.empty())
	{
		GetPrintBuffer().received.clear();
	}
}

