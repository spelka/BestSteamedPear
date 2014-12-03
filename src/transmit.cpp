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

	while (wConn.status != WConn::DEAD)
	{
		if (wConn.buffer_tx.empty()) continue;

		char response = ReadChar(wConn.TO2);


		//PrintToScreen(CHAT_LOG_TX, wConn.buffer_tx.size());

		PrintToScreen(CHAT_LOG_RX, "RESPONSE: N     RWEKJNRWEIOHRHWE"+ response);

		if (response == ACK)
		{
			PrintToScreen(CHAT_LOG_RX, "ACKERINOS");

			Transmit();
		}
		else //Timed out
		{
			PrintToScreen(CHAT_LOG_TX, "Timed Out");

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

	OVERLAPPED olap = { 0 };

	PrintToScreen(CHAT_LOG_TX, charToSend, false, true);

	return WriteFile(wConn.hComm, &charToSend, 1, NULL, &olap);
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
	OVERLAPPED olap = { 0 };
	
    char packet[PACKET_TOTAL_SIZE];

	GrapefruitPacket& gfp = wConn.buffer_tx.front();

	packet[0] = gfp.ctrl;
	packet[1] = gfp.sync;
	PrintToScreen(CHAT_LOG_TX, "Packet:");
	for (unsigned d = 0; d < PACKET_DATA_SIZE; ++d)
	{
		packet[d + 2] = gfp.data[d];
		PrintToScreen(CHAT_LOG_TX, "PACKET DATA" + (gfp.data[d]));
	}
	for (unsigned c = 0; c < PACKET_CRC_SIZE; ++c)
	{
		packet[PACKET_TOTAL_SIZE - PACKET_CRC_SIZE + c] = gfp.crc[c];
		stringstream pp;
		pp << "CRC DATA" << gfp.data;

		PrintToScreen(CHAT_LOG_TX, pp.str());
	}

	stringstream ss;
	ss << " muh dick" << string(packet);

	//MessageBox(NULL, ss.str().c_str(), "", MB_OK);


	bool fycj = WriteFile(wConn.hComm, packet, PACKET_TOTAL_SIZE, NULL, &olap);

	if(fycj)
		{
			//MessageBox(NULL, "penis" , "", MB_OK);
	}

	return fycj;
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

			PrintToScreen(CHAT_LOG_TX, "Before send!");
			SendPacket();
			PrintToScreen(CHAT_LOG_TX, "After send!");

			response = ReadChar(wConn.TO3);

			switch (response)
			{
			case ACK:
				PrintToScreen(CHAT_LOG_TX, "ACK response!");
				wConn.buffer_tx.pop_front();
				wConn.synFlip = !wConn.synFlip;
				break;

			case RVI:
				PrintToScreen(CHAT_LOG_TX, "RVI response!");
				wConn.canTransmit = !wConn.canTransmit;
				return;

			default:
				PrintToScreen(CHAT_LOG_TX, "Missed!");
				++missCount;
			}
		}
		while (response == NUL && missCount < MAX_MISS);
	}
	PrintToScreen(CHAT_LOG_TX, "Resetting state!");
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