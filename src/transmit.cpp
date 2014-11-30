/*
GLOBAL VARIABLES TO2, TO3
Booleans: stopWaiting, ackReceived, rviState
//Transmit Thread
stopWaiting = ackReceived = rviState = false
Send ENQ
//Send ENQ
Set a timer that has the length of TO2 which calls the Reset State function when it ends
while ACK has not been received and stopWaiting is false
if the software receives an ACK
ackReceived = true
Transmit Mode
if rviState
return
else
Reset State
//Transmit Mode
while there is more data AND send_count is less than max_send
In the case that Send Data returns an ACK:
more data to send, increment send_count
In the case that Send Data returns an NACK:
resend data
timeoutCount++
In the case that Send Data returns an RVI:
rviState = true
//Send Data
Take data out of buffer
add crc to data
send
Wait for response
Return response
//Reset State
stopWaiting = true
if !ackReceived
Sleep(TO3 + a randomly generated number)
*/

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
	for (unsigned send = 0; send < MAX_SEND; ++send)
	{
		if (GetWConn().buffer_send.size() <= 0) return;

		char response = SendPacket();

		if (response == RVI)
			GetWConn().canTransmit = !GetWConn().canTransmit;
	}
}

void ResetState()
{
	Timer::WaitForResponse(GetWConn().TO4);
}