#include <deque>
#include "receive.h"
#include "transmit.h"
#include "protocol.h"
#include "crc.h"
#include "application.h"

using namespace std;
bool receivingMode = false;
COMMTIMEOUTS timeouts = { 0, 0, 0, 0, 0 };

bool SyncTracker::firstSync = true;
char SyncTracker::previousSync = true;

bool SyncTracker::CheckSync(char syncbit)
{
	if (!firstSync)
	{

		if (previousSync != syncbit)
		{
			previousSync = syncbit;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		firstSync = false;
		previousSync = syncbit;
	}
    return true;
}


void SyncTracker::FlagForReset()
{
	firstSync = true;
}

//Comment
DWORD WINAPI ReceiveThread(LPVOID lpvThreadParm)
{
	WConn& wConn = GetWConn();

	FillRxBuffer();

	wConn.buffer_rx.clear();

	return NULL;
}

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
	WConn& wConn = GetWConn();
	
	timeouts.ReadIntervalTimeout = timeout;
	char received = NUL;
	// set timeouts
	if (!SetCommTimeouts(wConn.hComm, &timeouts))
	{
		// Error setting time-outs.
	}

	//read in character from comm port
	//if you fail to read in a file, return NUL
	ReadFile(wConn.hComm, &received, 1, NULL, &wConn.olap);

	if (received != NUL) PrintToScreen(CHAT_LOG_RX, received);
	
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
	WConn& wConn = GetWConn();

	char controlChar;
	char buffer[PACKET_SIZE];
	char syncBit;
	DWORD dwCommEvent;
	DWORD dwRead = 0;
	deque<char>& netBuf = wConn.buffer_rx;

	//if the comm mask is successfully set to watch for receiving character events
	if (!SetCommMask(wConn.hComm, EV_RXCHAR))
	{
		return false;
	}
    
	timeouts.ReadIntervalTimeout = wConn.TO1;
	// set timeouts
	if (!SetCommTimeouts(wConn.hComm, &timeouts))
	{
		// Error setting time-outs.
	}

	for (;;)
	{
		bool packetRead = false;

		if (WaitCommEvent(wConn.hComm, &dwCommEvent, NULL))
		{
			do
			{
				if (ReadFile(wConn.hComm, &controlChar, 1, NULL, &osRead))
				{
					PrintToScreen(CHAT_LOG_RX, string("148) ") + controlChar);

					//if the data in the buffer is a packet
					if (controlChar == EOT || controlChar == ETB)
					{
						//check sync bits
						if (ReadFile(wConn.hComm, &syncBit, 1, NULL, &osRead))
						{
							PrintToScreen(CHAT_LOG_RX, string("156) ") + controlChar);

							//if the sync bit is OK
							if (SyncTracker::CheckSync(syncBit))
							{
								wConn.buffer_rx.push_back(controlChar);
								wConn.buffer_rx.push_back(syncBit);

								//if you successfully read the packet in
								if (ReadFile(wConn.hComm, &dwRead, PACKET_SIZE - 2, NULL, &osRead))
								{
									//push the characters into the receive buffer
									for (unsigned int i = 1; i < PACKET_SIZE; i++)
									{
										wConn.buffer_rx.push_back(buffer[i]);
									}

									packetRead = true;
								}
							}
						}
						//if EOT, reset Sync Bit
						if (controlChar == EOT)
						{
							SyncTracker::FlagForReset();
						}
					}
					else if (controlChar == ENQ)
					{
						SendChar(ACK, 0);
					}
				}
				else
				{
					//an error occured while reading in the file
					break;
				}
			} while (dwRead);

			if (packetRead)
			{
				if (ValidateData())
				{
					TrimPacket();
					
					while (!netBuf.empty()) //clear buffer
					{
						PrintToScreen(CHAT_LOG_RX, netBuf.front());
						netBuf.pop_front();
					}

					SendChar(ACK, 0);
				}
				else
				{
					SendChar(NAK, 0);
				}
			}
		}
		else
		{
			//error in WaitCommEvent
			break;
		}
	}
	return true;
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
bool ValidateData()
{
    WConn w = GetWConn();

	if (w.buffer_rx.empty()) return false;

    unsigned long crcResult = crc( w.buffer_rx.begin() + 2, w.buffer_rx.begin() + 3 + PACKET_SIZE );

    deque<char>::iterator it = w.buffer_rx.begin() + 3 + PACKET_SIZE;
    for( int i = 0; i < 4; ++i )
    {
        if( (char)crcResult != *it )
            return false;
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

void TrimPacket()
{
    deque<char> netBuf = wConn.buffer_rx;
    netBuf.pop_front();
    netBuf.pop_front();

	deque<char>::iterator packetIterator = netBuf.begin();

	for (unsigned int i = 0; i < 1018; i++)
	{
		if ((*packetIterator) == ETX)
		{
            break;
		}
		packetIterator++;
	}
    netBuf.erase(packetIterator, netBuf.end());


}