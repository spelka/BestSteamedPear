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

bool stopWaiting = false;
bool rviState = false;

HANDLE hCommPort;
UINT_PTR IDT_SENDENQTIMER;
bool responseReceived;
bool timeOut;

DWORD WINAPI TransmitThread(LPVOID lpvThreadParm)
{
	responseReceived = false;
	return NULL;
}

VOID CALLBACK MyTimerProc(
	HWND hwnd,        // handle to window for timer messages 
	UINT message,     // WM_TIMER message 
	UINT idTimer,     // timer identifier 
	DWORD dwTime)     // current system time 
{
	if (idTimer == IDT_SENDENQTIMER)
	{
		timeOut = true;
	}
}

void SendENQ()
{
	SetTimer(NULL,                // handle to main window 
		IDT_SENDENQTIMER,         // timer identifier 
		GetWConn().TO2,           // timeout 2
		(TIMERPROC)MyTimerProc);  // timer callback

	//While ack has not been received and timeout is not true
	while (!responseReceived && !timeOut)
	{
		//set receivedChar empty
		if (ReceiveChar(ACK))
		{
			responseReceived = true;
		}
	}

	if (responseReceived)
	{
		Transmit();
	}
	else //Timed out
	{
		ResetState();
	}
}

char SendData()
{
	char response = NUL;



	return response;
}

void ResetState()
{

}