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

	wConn.buffer_rx_packet.clear();

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

	if (wConn.buffer_rx_ctrl.empty()) return NUL;

	char ctrl = wConn.buffer_rx_ctrl.front();

	wConn.buffer_rx_ctrl.pop_front();

	PrintToScreen(CHAT_LOG_RX, ctrl, false, true);
	
	return ctrl;
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

	DWORD nBytesRead, dwEvent, dwError;
	COMSTAT cs;

	//if the comm mask is successfully set to watch for receiving character events
	if (!SetCommMask(wConn.hComm, EV_RXCHAR))
	{
		return false;
	}
    
	timeouts.ReadIntervalTimeout = wConn.TO1;
	// set timeouts
	if (!SetCommTimeouts(wConn.hComm, &timeouts))
	{
		return false;
	}

	while (wConn.isConnected)
	{
		GrapefruitPacket g;
		bool packetRead = false;

		if (WaitCommEvent(wConn.hComm, &dwEvent, NULL))
		{
			ClearCommError(wConn.hComm, &dwError, &cs);

			do
			{
				if (ReadFile(wConn.hComm, &g.ctrl, 1, &nBytesRead, NULL))
				{
					//if the data in the buffer is a packet
					if (g.ctrl == EOT || g.ctrl == ETB)
					{
						//check sync bits
						if (ReadFile(wConn.hComm, &g.sync, 1, &nBytesRead, NULL))
						{
							//if the sync bit is OK
							if (SyncTracker::CheckSync(g.sync))
							{
								//if you successfully read the packet in
								if (ReadFile(wConn.hComm, g.data, PACKET_DATA_SIZE, &nBytesRead, NULL))
								{
									packetRead = true;
									PrintToScreen(CHAT_LOG_RX, string(reinterpret_cast<char*>(g.data)), false, true);
								}
							}
						}
						//if EOT, reset Sync Bit
						if (g.ctrl == EOT)
						{
							SyncTracker::FlagForReset();
						}
					}
					else
					{
						wConn.buffer_rx_ctrl.push_back(g.ctrl);

						if (g.ctrl == ENQ)
						{
							SendChar(ACK);
						}
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
					PrintToScreen(CHAT_LOG_RX, string(reinterpret_cast<char*>(g.data)), false, true);
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

	PurgeComm(wConn.hComm, PURGE_RXCLEAR);

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