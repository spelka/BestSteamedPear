#include "receive.h"
#include "protocol.h"

/*
GLOBAL VARIABLES TO2, TO3

Booleans : stopWaiting, ackReceived, rviState

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
In the case that Send Data returns an ACK :
more data to send, increment send_count

In the case that Send Data returns an NACK :
resend data
timeoutCount++

In the case that Send Data returns an RVI :
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

//receieve.cpp
bool receivingMode = false;

//receive an ENQ or RVI
//send ACK
//go into a receiving loop
//if you timeout
//timeout += TO1
//if timeout == TO3 * MAX_MISS
//return to idle protocol
//if you receive a packet
//CRC validate the packet
//if the packet is valid and ETB
//return to receiving loop
//else if the packet is invalid and miss < MAX_MISS
//return to receiving loop
//else if the packet is valid and EOT
//print message to screen
//return to idle protocol
//else if you need to send
//send RVI
//on ACK of RVI, go to transmission thread


void SendACK(HANDLE hComm)
{
	//Send an ACK
	if (!WriteFile(hComm, &ACK, 1, NULL, NULL))
	{
		MessageBox(NULL, TEXT("Error: Failed to send ACK"), TEXT("Error"), MB_OK | MB_ICONERROR);
	}

	return;
}

//receives a character from the comm port and checks if the received char is what was expected
bool ReceiveChar(char expectedChar)
{
	char received;

	//read in character from comm port
	//if you fail to read in a file, return false
	if (!ReadFile(GetWConn().hComm, &received, 1, NULL, NULL))
	{
		return false;
	}
	//if read was successful, compare the read in character to the expected character
	if (expectedChar == received)
	{
		return true;
	}
	return false;
}

//receives a character from the comm port and returns it
char ReceiveChar()
{
	char received;

	//read in character from comm port
	//if you fail to read in a file, return false
	if (!ReadFile(GetWConn().hComm, &received, 1, NULL, NULL))
	{
		return NUL;
	}
	//if read was successful, return the read-in character
	return received;
}


void receivePacket(WConn* w)
{
	//read in data from port to the WConn->buffer_receive
}

void invalidData()
{

}

void packetValidator()
{

}

void timeOut1()
{

}

void validDataEOT()
{

}

void validDataETB()
{

}