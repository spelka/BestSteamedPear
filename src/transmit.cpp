/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		trasmit.cpp
--
-- PROGRAM:			BestSteamPear
--
-- FUNCTIONS:		DWORD WINAPI TransmitThread(LPVOID);
--					char SendChar(char charToSend, unsigned toDuration);
--					char SendPacket();
--					void Transmit();
--					void ResetState();
--
-- DATE: 			November 29, 2014
--
-- REVISIONS: 		NONE
--
-- DESIGNER: 		Melvin Loho, Alex Lam, Sebastian Pelka, Georgi Hristov
--
-- PROGRAMMER: 		Melvin Loho, Alex Lam
--
-- NOTES:
-- This is the code that takes care of all the trasmitting that is done in this program.
----------------------------------------------------------------------------------------------------------------------*/

#include "transmit.h"
#include "protocol.h"
#include "receive.h"
#include "application.h"
#include "crc.h"

#include <algorithm>

using namespace std;


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTIONS:		TransmitThread
--
-- DATE: 			November 29, 2014
--
-- REVISIONS: 		NONE
--
-- DESIGNER: 		Melvin Loho, Alex Lam, Sebastian Pelka, Georgi Hristov
--
-- PROGRAMMER: 		Melvin Loho
--
-- NOTES:
-- This begins the thread for sending data.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI TransmitThread(LPVOID lpvThreadParm)
{
	WConn& wConn = GetWConn();

	while (wConn.isConnected)
	{
		if (wConn.buffer_tx.empty()) continue;

		SendChar(ENQ);

		char response = ReadChar(wConn.TO2);

		//PrintToScreen(CHAT_LOG_RX, response);

		//PrintToScreen(CHAT_LOG_TX, wConn.buffer_tx.size());

		if (response != NUL)
		{
			Transmit();
		}
		else //Timed out
		{
			ResetState();
		}
	}

	wConn.buffer_tx.clear();

	return NULL;
}

void Packetize(std::string s)
{
	WConn& wConn = GetWConn();

	while (!s.empty())
	{
		GrapefruitPacket gfp;
		unsigned currDataSize = min(PACKET_DATA_SIZE, s.size());
		unsigned long crcResult;

		gfp.ctrl = s.size() > PACKET_DATA_SIZE ? ETB : EOT;
		gfp.sync = wConn.synFlip;

		for (unsigned p = 0; p < PACKET_DATA_SIZE; ++p)
		{
			gfp.data[p] = PAD;
		}

		for (unsigned d = 0; d < currDataSize; ++d)
		{
			gfp.data[d] = s[d];
		}

		s = s.substr(currDataSize);

		if (currDataSize < PACKET_DATA_SIZE) gfp.data[currDataSize] = ETX;

		crcResult = crc(gfp.data, PACKET_DATA_SIZE);

		gfp.crc[0] = (char)(crcResult >> 24);
		gfp.crc[1] = (char)(crcResult >> 16);
		gfp.crc[2] = (char)(crcResult >> 8);
		gfp.crc[3] = (char)(crcResult >> 0);

		wConn.buffer_tx.emplace_back(gfp);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTIONS:		SendChar
--
-- DATE: 			November 29, 2014
--
-- REVISIONS: 		NONE
--
-- DESIGNER: 		Melvin Loho, Alex Lam, Sebastian Pelka, Georgi Hristov
--
-- PROGRAMMER: 		Melvin Loho
--
-- NOTES:
-- This function is used by different parts of the program to send characters(mostly control characters)
----------------------------------------------------------------------------------------------------------------------*/
bool SendChar(char charToSend)
{
	WConn &wConn = GetWConn();

	PrintToScreen(CHAT_LOG_TX, charToSend, false, true);

	return WriteFile(wConn.hComm, &charToSend, 1, NULL, &wConn.olap);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTIONS:		SendPacket
--
-- DATE: 			November 29, 2014
--
-- REVISIONS: 		NONE
--
-- DESIGNER: 		Melvin Loho, Alex Lam, Sebastian Pelka, Georgi Hristov
--
-- PROGRAMMER: 		Melvin Loho, Alex Lam
--
-- NOTES:
-- This is used specifically by Tramsit() to send packets of data.
-- Copies the vector, appends appropiate padding ,contorl characters, crc and writes to comm port.
----------------------------------------------------------------------------------------------------------------------*/
bool SendPacket()
{
	WConn &wConn = GetWConn();
	
	char packet[1024];

	GrapefruitPacket& gfp = wConn.buffer_tx.front();

	packet[0] = gfp.ctrl;
	packet[1] = gfp.sync;
	for (unsigned d = 0; d < PACKET_DATA_SIZE; ++d)
	{
		packet[d + 2] = gfp.data[d];
	}
	for (unsigned c = 0; c < PACKET_CRC_SIZE; ++c)
	{
		packet[PACKET_TOTAL_SIZE - PACKET_CRC_SIZE + c] = gfp.crc[c];
	}

	return WriteFile(wConn.hComm, packet, PACKET_DATA_SIZE, NULL, &wConn.olap);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTIONS:		Transmit
--
-- DATE: 			November 29, 2014
--
-- REVISIONS: 		NONE
--
-- DESIGNER: 		Melvin Loho, Alex Lam, Sebastian Pelka, Georgi Hristov
--
-- PROGRAMMER: 		Alex Lam
--
-- NOTES:
-- This is the trasmit state, it will send data if the buffer_tx is not empty 
-- continue to send data until MAX_MISS or MAX_SEND has been reached.
----------------------------------------------------------------------------------------------------------------------*/
void Transmit()
{
	WConn &wConn = GetWConn();

	if (wConn.buffer_tx.empty()) return;

	int missCount = 0;

	for (unsigned send = 0; send < MAX_SEND; ++send)
	{
		char response = NUL;

		do
		{
			if (wConn.buffer_tx.empty()) break;

			SendPacket();

			response = ReadChar(wConn.TO3);

			switch (response)
			{
			case ACK:
				wConn.buffer_tx.pop_front();
				wConn.synFlip = !wConn.synFlip;
				break;

			case RVI:
				wConn.canTransmit = !wConn.canTransmit;
				return;

			default:
				++missCount;
			}
		}
		while (response == NUL && missCount < MAX_MISS);
	}

	ResetState();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTIONS:		Transmit
--
-- DATE: 			November 29, 2014
--
-- REVISIONS: 		NONE
--
-- DESIGNER: 		Melvin Loho, Alex Lam, Sebastian Pelka, Georgi Hristov
--
-- PROGRAMMER: 		Alex Lam
--
-- NOTES:
-- This is the reset state, it randomizes the timeout before returning to idle to avoid collisions from going to idle to 
-- send.
----------------------------------------------------------------------------------------------------------------------*/
void ResetState()
{
	Sleep(GetWConn().TO4);
}