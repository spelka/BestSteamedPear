#include <deque>
#include "receive.h"
#include "protocol.h"
#include "crc.h"

using namespace std;

/*
GLOBAL VARIABLES TO2, TO3
Booleans : stopWaiting, ackReceived, rviState
//Transmit Thread
stopWaiting = ackReceived = rviState = false
Send ENQ
//Send ENQ
Set a timer that has the length of TO2 which calls the Reset State function when it ends
while ACK has not been received and stopWaiting is false
if the software receives an ACK
ackReceived = true
Transmit Mode
if rviState
return
else
Reset State
//Transmit Mode
while there is more data AND send_count is less than max_send
In the case that Send Data returns an ACK :
more data to send, increment send_count
In the case that Send Data returns an NACK :
resend data
timeoutCount++
In the case that Send Data returns an RVI :
rviState = true
//Send Data
Take data out of buffer
add crc to data
send
Wait for response
Return response
//Reset State
stopWaiting = true
if !ackReceived
Sleep(TO3 + a randomly generated number)
*/

//receieve.cpp
bool receivingMode = false;

//receive an ENQ or RVI
//send ACK
//go into a receiving loop
//if you timeout
//timeout += TO1
//if timeout == TO3 * MAX_MISS
//return to idle protocol
//if you receive a packet
//CRC validate the packet
//if the packet is valid and ETB
//return to receiving loop
//else if the packet is invalid and miss < MAX_MISS
//return to receiving loop
//else if the packet is valid and EOT
//print message to screen
//return to idle protocol
//else if you need to send
//send RVI
//on ACK of RVI, go to transmission thread

COMMTIMEOUTS timeouts = { 0, 0, 0, 0, 0 };

//---------------------------------------------------------------------------------------------
//receives a character from the comm port and checks if the received char is what was expected
//
//Created On: Wednesday, November 19
//
//Designed By: Sebastian Pelka
//
//Modified By:	Georgi Hristov, November 29, 2014
//				Sebastian Pelka, November 29, 2014
//
//---------------------------------------------------------------------------------------------
char ReadChar(WConn& w, DWORD timeout)
{
	timeouts.ReadIntervalTimeout = timeout;
	char received = NUL;
	// set timeouts
	if (!SetCommTimeouts(w.hComm, &timeouts))
	{
		// Error setting time-outs.
	}

	//read in character from comm port
	//if you fail to read in a file, return NUL
	ReadFile(GetWConn().hComm, &received, 1, NULL, NULL);
	
	return received;
}

//---------------------------------------------------------------------------
// This function is designed to receive packets coming in on the serial port. 
// It is event-driven, and operates when a recieve char event occurs on the 
// port. As a result, this is a blocking funciton and should therefore be run 
// in its own thread.
//
// Source: http://msdn.microsoft.com/en-us/library/ff802693.aspx
//
//Created On: Wednesday, November 19
//
//Designed By: Sebastian Pelka
//
//Modified By:	Georgi Hristov, November 29, 2014
//				Sebastian Pelka, November 29, 2014
//
//---------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool FillRxBuffer(WConn& w)
{
	char controlChar;
	char buffer[PACKET_SIZE];
	DWORD dwCommEvent;
	DWORD dwRead = 0;

	//if the comm mask is successfully set to watch for receiving character events
	if (!SetCommMask(w.hComm, EV_RXCHAR))
	{
		return false;
	}

	for (;;)
	{
		if (WaitCommEvent(w.hComm, &dwCommEvent, NULL))
		{
			do
			{
				if (ReadFile(w.hComm, &controlChar, 1, NULL, NULL))
				{
					//if the data in the buffer is a packet
					if (controlChar == EOT || controlChar == ETB)
					{
						//if you successfully read the packet in
						if (ReadFile(w.hComm, &dwRead, PACKET_SIZE - 1, NULL, NULL))
						{
							//push the characters into the receive buffer
							for (unsigned int i = 1; i < PACKET_SIZE; i++)
							{
								(w.buffer_receive).push_back(buffer[i]);
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

void invalidData()
{

}

void validateData()
{

}

//-------------------------------------------------------------------------------------------------
// Iterates over the WConn reveived buffer if the CRC validator confirmed the received data is good.
// This function pulls the data out of the packet structure and adds it to the print buffer, which
// contains all validated message data so far.
//
// Returns true if the ETX character is found.
//
// Created On: November 29, 2014 by Sebastian Pelka
//
//--------------------------------------------------------------------------------------------------
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

