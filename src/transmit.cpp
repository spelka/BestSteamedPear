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

#include <vector>
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

	PrintToScreen(CHAT_LOG_TX, charToSend);

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
	deque<char> packet;
	unsigned long crcResult;
	char first, second, third, fourth;

	packet.insert(
		packet.end(),

		wConn.buffer_tx.begin(),

		/*if*/     wConn.buffer_tx.size() >= PACKET_DATA_SIZE
		/*true*/   ? wConn.buffer_tx.begin() + PACKET_DATA_SIZE
		/*false*/  : wConn.buffer_tx.begin() + wConn.buffer_tx.size()
		);

	if (packet.size() < PACKET_DATA_SIZE) packet.push_back(ETX);

	//Pads the word until packet data size
	for (unsigned p = 0; p < PACKET_DATA_SIZE - packet.size(); ++p)
	{
		packet.push_back(PAD);
	}

	//CRC's the word
	crcResult = crc(packet.begin(), packet.end());

	first = (char)(crcResult >> 24);
	second = (char)(crcResult >> 16);
	third = (char)(crcResult >> 8);
	fourth = (char)(crcResult >> 0);

	//Pushes the crc to the word
	packet.push_back(first);
	packet.push_back(second);
	packet.push_back(third);
	packet.push_back(fourth);

	//Appends the control characters needed for the word
	packet.push_front((int)wConn.synFlip);
	if (wConn.buffer_tx.size() > PACKET_DATA_SIZE)
	{
		packet.push_front(ETB);
	}
	else
	{
		packet.push_front(EOT);
	}

	PrintToScreen(CHAT_LOG_TX, string("\nPacket Sent!"));
	/*
	unsigned wrap = 0;
	for (char c : packet)
	{
		if (++wrap % 80 == 0) PrintToScreen(CHAT_LOG_TX, '\n');
		PrintToScreen(CHAT_LOG_TX, c);
	}
	PrintToScreen(CHAT_LOG_TX, "");
	*/

	return WriteFile(wConn.hComm, &packet, PACKET_DATA_SIZE, NULL, &wConn.olap);
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

		do {

			if (wConn.buffer_tx.empty()) break;

			SendPacket();

			response = ReadChar(wConn.TO3);

			switch (response)
			{
			case ACK:
				PrintToScreen(CHAT_LOG_TX, response);

				wConn.buffer_tx.erase(wConn.buffer_tx.begin(),
					wConn.buffer_tx.begin() + PACKET_DATA_SIZE - 1);
				wConn.synFlip = !wConn.synFlip;
				break;

			case RVI:
				wConn.canTransmit = !wConn.canTransmit;
				return;

			default:
				++missCount;
			}

		} while (response == NUL && missCount < MAX_MISS);
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