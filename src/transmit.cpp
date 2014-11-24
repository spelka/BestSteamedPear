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

bool stopWaiting = false;
bool ackReceived = false;
bool rviState = false;

HANDLE hCommPort;

DWORD WINAPI TransmitThread(LPVOID lpvThreadParm)
{
	return NULL;
}

void SendENQ()
{
	bool ackReceived;
	bool timeOut;
	char receivedChar;

	/*
	SetTimer(hwnd,                // handle to main window 
		IDT_SENDENQTIMER,         // timer identifier 
		5000,                     // 5-second interval 
		(TIMERPROC)MyTimerProc);  // timer callback
		*/

	//While ack has not been received and timeout is not true
	while (!ackReceived && !timeOut)
	{
		//set receivedChar empty
		ReadFile(hCommPort, &receivedChar, 1, NULL, NULL);
		if (receivedChar == ACK)
		{
			ackReceived = true;
		}
		else
		{
			//increment timeout
			/*
			if (timeoutCounter > TO2)
			{
				//timeoutCounter = true;
			}*/
		}
	}
	if (ackReceived)
	{
		WriteFile(hCommPort, &ENQ, 1, NULL, NULL);
	}
	else //Timed out
	{
		resetState();
	}
}

char sendData()
{
	char response;



	return response;
}

void resetState()
{
	
}

