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

	char buffer[PACKET_TOTAL_SIZE];
	DWORD dwCommEvent;
	DWORD dwRead = 0;

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
		GrapefruitPacket g;
		bool packetRead = false;

		if (WaitCommEvent(wConn.hComm, &dwCommEvent, NULL))
		{
			do
			{
				if (ReadFile(wConn.hComm, &g.ctrl, 1, NULL, &GetWConn().olap))
				{
					//if the data in the buffer is a packet
					if (g.ctrl == EOT || g.ctrl == ETB)
					{
						//check sync bits
						if (ReadFile(wConn.hComm, &g.sync, 1, NULL, &GetWConn().olap))
						{
							//if the sync bit is OK
							if (SyncTracker::CheckSync(g.sync))
							{

								//if you successfully read the packet in
								if (ReadFile(wConn.hComm, &g.data, PACKET_DATA_SIZE, NULL, &GetWConn().olap))
								{
									packetRead = true;
								}
							}
						}
						//if EOT, reset Sync Bit
						if (g.ctrl == EOT)
						{
							SyncTracker::FlagForReset();
						}
					}
					else if (g.ctrl == ENQ)
					{
						SendChar(ACK);
					}
				}
				else
				{
					//an error occured while reading in the file
					break;
				}
			} while (g.data);
			
			//if a packet was read in
			if (packetRead)
			{
				//if the packet is successfully validated
				if (ValidateData(g))
				{
					stringstream ss;
					ss << g.data;
					PrintToScreen(CHAT_LOG_RX, ss.str());
					SendChar(ACK);
				}
				else
				{
					SendChar(NAK);
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
bool ValidateData(GrapefruitPacket g)
{
    WConn w = GetWConn();
	stringstream ss;

	if (g.data == NULL)
	{
		return false;
	}

	crcInit();
	unsigned long crcResult = crc(g.data, PACKET_DATA_SIZE);

	ss << crcResult;

	unsigned char chararray[4];
	ss >> chararray;

	if (chararray == g.data)
	{
		return true;
	}

	return false;
}