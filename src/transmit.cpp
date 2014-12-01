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
	for (;;)
	{
		if (GetWConn().buffer_send.empty()) continue;

		SendChar(ENQ, GetWConn().TO2);

		char response = ReadChar(GetWConn().TO2);

		//PrintToScreen(CHAT_LOG_RX, response);

		//PrintToScreen(CHAT_LOG_TX, GetWConn().buffer_send.size());

		if (response != NUL)
		{
			Transmit();
		}
		else //Timed out
		{
			ResetState();
		}
	}

	GetWConn().buffer_send.clear();

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
bool SendChar(char charToSend, unsigned toDuration)
{
	OVERLAPPED osWrite = { 0 };

	PrintToScreen(CHAT_LOG_TX, charToSend);

	return WriteFile(GetWConn().hComm, &charToSend, 1, NULL, &osWrite);
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
	char response;
	deque<char> packet;
	unsigned long crcResult;
	char first, second, third, fourth;
	deque<char> paddedPacket;

	OVERLAPPED osWrite = { 0 };

	packet.insert(
		packet.end(),

		GetWConn().buffer_send.begin(),

		/*if*/     GetWConn().buffer_send.size() >= PACKET_DATA_SIZE
		/*true*/   ? GetWConn().buffer_send.begin() + PACKET_DATA_SIZE - 1
		/*false*/  : GetWConn().buffer_send.begin() + GetWConn().buffer_send.size() - 1
		);

	packet.push_back(ETX);

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
	if (GetWConn().buffer_send.size() > PACKET_DATA_SIZE)
	{
		paddedPacket.push_back(ETB);
	}
	else
	{
		paddedPacket.push_back(EOT);
	}
	paddedPacket.push_back((int)GetWConn().synFlip);
	paddedPacket.insert(paddedPacket.begin(), packet.begin(), packet.end());

	PrintToScreen(CHAT_LOG_TX, string("\nPacket:"));
	unsigned wrap = 0;
	for (char c : paddedPacket)
	{
		if (++wrap % 80 == 0) PrintToScreen(CHAT_LOG_TX, '\n');
		PrintToScreen(CHAT_LOG_TX, c);
	}
	PrintToScreen(CHAT_LOG_TX, "");

	return WriteFile(GetWConn().hComm, &paddedPacket, PACKET_DATA_SIZE, NULL, &osWrite);
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
-- This is the trasmit state, it will send data if the buffer_send is not empty 
-- continue to send data until MAX_MISS or MAX_SEND has been reached.
----------------------------------------------------------------------------------------------------------------------*/
void Transmit()
{
	if (GetWConn().buffer_send.empty()) return;

	int missCount = 0;

	for (unsigned send = 0; send < MAX_SEND; ++send)
	{
		char response = NUL;

		do {

			if (GetWConn().buffer_send.empty()) break;

			SendPacket();

			response = ReadChar(GetWConn().TO3);

			switch (response)
			{
			case ACK:
				PrintToScreen(CHAT_LOG_TX, response);

				GetWConn().buffer_send.erase(GetWConn().buffer_send.begin(),
					GetWConn().buffer_send.begin() + PACKET_DATA_SIZE - 1);
				GetWConn().synFlip = !GetWConn().synFlip;
				break;

			case RVI:
				GetWConn().canTransmit = !GetWConn().canTransmit;
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