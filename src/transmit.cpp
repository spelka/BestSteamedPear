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

char SendChar(char charToSend, unsigned toDuration)
{
	if (!WriteFile(GetWConn().hComm, &charToSend, 1, NULL, NULL))
	{
		return NUL;
	}

	return Timer::WaitForResponse(GetWConn().TO2);
}

char SendPacket()
{
	char response;
	vector<char> packet;

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
		break;
	}

	return response;
}

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

void ResetState()
{
	Timer::WaitForResponse(GetWConn().TO4);
}