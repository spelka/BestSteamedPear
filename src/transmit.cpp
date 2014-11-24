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

bool stopWaiting = false;
bool ackReceived = false;
bool rviState = false;

HANDLE hFile;

DWORD WINAPI TransmitThread(LPVOID lpvThreadParm)
{
	
}

void SendENQ()
{
	ReadFile(hFile, &d, 1, &x, NULL);

}

char sendData()
{
	char response;



	return response;
}

void resetState()
{
	
}

