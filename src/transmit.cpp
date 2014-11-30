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
	char response = SendChar(ENQ, GetWConn().TO2);

	if (response != NUL)
	{
		Transmit();
	}
	else //Timed out
	{
		ResetState();
	}

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
char SendChar(char charToSend, unsigned toDuration)
{
	if (!WriteFile(GetWConn().hComm, &charToSend, 1, NULL, NULL))
	{
		return NUL;
	}

	return Timer::WaitForResponse(GetWConn().TO2);
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
char SendPacket()
{
	char response;
	vector<char> packet;
	vector<char> paddedPacket;

	copy(
		GetWConn().buffer_send.begin(),

		GetWConn().buffer_send.size() >= PACKET_DATA_SIZE
		? GetWConn().buffer_send.begin() + PACKET_DATA_SIZE - 1
		: GetWConn().buffer_send.begin() + GetWConn().buffer_send.size() - 1,

		packet.begin()
		);

	packet.push_back(ETX);

	for (unsigned p = 0; p < PACKET_DATA_SIZE - packet.size(); ++p)
	{
		packet.push_back(PAD);
	}

	/*
	CRC packet 
	*/

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

	if (!WriteFile(GetWConn().hComm, &packet, PACKET_DATA_SIZE, NULL, NULL))
	{
		return NUL;
	}

	response = Timer::WaitForResponse(GetWConn().TO3);

	switch (response)
	{
	case ACK:
		GetWConn().buffer_send.erase(GetWConn().buffer_send.begin(),
			GetWConn().buffer_send.begin() + PACKET_DATA_SIZE - 1);
		GetWConn().synFlip = !GetWConn().synFlip;
		break;
	}

	return response;
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
	int missCount = 0;

	for (unsigned send = 0; send < MAX_SEND; ++send)
	{
		if (GetWConn().buffer_send.size() <= 0) return;

		//Try again if receiver replies NULL or NAK
		char response = SendPacket();
		while (response == NUL && missCount < MAX_MISS)
		{
			missCount++;
			char response = SendPacket();
		}

		//If receiver replies with RVI
		if (response == RVI)
		{
			GetWConn().canTransmit = !GetWConn().canTransmit;
			break;
		}
	}
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
	Timer::WaitForResponse(GetWConn().TO4);
}