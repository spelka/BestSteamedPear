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

	while (wConn.status == WConn::IDLE)
	{
        if( !ReadChar( ENQ, 5000 ) ) continue;
		PrintToScreen(CHAT_LOG_RX, ENQ, false, true);
        SendChar( ACK );
        do
        {
            ReadPacket();
            if( ValidatePacket( wConn.buffer_rx_packet.front() ) )
            {
                SendChar( ACK );
            }
            else
            {
                SendChar( NAK );
            }

        }
        while( !wConn.buffer_rx_ctrl.empty()
            && wConn.buffer_rx_packet.front().ctrl == ETB );
	}

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

	DWORD nBytesRead, dwError;
	COMSTAT cs;
	char ctrl = NUL;

	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = timeout;

	// set timeouts
	if (!SetCommTimeouts(wConn.hComm, &timeouts))
	{
		return NUL;
	}

	ClearCommError(wConn.hComm, &dwError, &cs);

	if (!ReadFile(wConn.hComm, &ctrl, 1, &nBytesRead, NULL))
	{
		return NUL;
	}

	return ctrl;
}

bool ReadChar(char expectedChar, DWORD timeout)
{
	return ReadChar(timeout) == expectedChar;
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
bool ReadPacket()
{
	WConn& wConn = GetWConn();
    
    char buffer[PACKET_DATA_SIZE];

	DWORD nBytesRead, dwEvent, dwError;
	COMSTAT cs;

	//if the comm mask is successfully set to watch for receiving character events
	if (!SetCommMask(wConn.hComm, EV_RXCHAR))
	{
		return false;
	}

	timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant = wConn.TO1;

	// set timeouts
	if (!SetCommTimeouts(wConn.hComm, &timeouts))
	{
		return false;
	}

	GrapefruitPacket g;
	bool packetRead = false;

	ClearCommError(wConn.hComm, &dwError, &cs);

    if (packetRead = ReadFile(wConn.hComm, buffer, PACKET_TOTAL_SIZE, &nBytesRead, NULL))
	{
           g.ctrl = buffer[0];
           g.sync = buffer[1];
           for( int i = 0; i < PACKET_DATA_SIZE; ++i )
               g.data[i] = buffer[i + 2];
           for( int i = 0; i < PACKET_CRC_SIZE; ++i )
               g.crc[i] = buffer[i + 2 + PACKET_DATA_SIZE];

           wConn.buffer_rx_packet.emplace_back( g );

	    //if (SyncTracker::CheckSync(g.sync))
	    //{
	    //	//if you successfully read the packet in
	    //	if (ReadFile(wConn.hComm, g.data, PACKET_DATA_SIZE, &nBytesRead, NULL))
	    //	{
	    //		packetRead = true;
	    //		PrintToScreen(CHAT_LOG_RX, string(reinterpret_cast<char*>(g.data)), false, true);
	    //	}
	    //}
		//if (g.ctrl == EOT)
		//{
		//    SyncTracker::FlagForReset();
		//}
	}

	return packetRead;
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
bool ValidatePacket(GrapefruitPacket g)
{
    WConn w = GetWConn();
	stringstream ss;

	if (g.data == NULL)
	{
		return false;
	}

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