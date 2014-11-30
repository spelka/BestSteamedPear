#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <windows.h>
#include <deque>

const char NUL = 0x00;
const char ETX = 0x03;
const char EOT = 0x04;
const char ENQ = 0x05;
const char ACK = 0x06;
const char RVI = 0x17;
const char NAK = 0x21;
const char ETB = 0x23;
const char PAD = 0x54;

const unsigned PACKET_SIZE = 1024; //packet size in bytes
const unsigned PACKET_DATA_SIZE = 1018; //packet size in bytes

const unsigned MAX_SEND = 10;
const unsigned MAX_MISS = 10;

//////

struct WConn
{
	HANDLE hComm;

	unsigned
		TO1,
		TO2,
		TO3,
		TO4
		;

	bool canTransmit;

	std::deque<char> buffer_receive;
	std::deque<char> buffer_send;

	bool synFlip;
};

//////

//the contents of these buffers are meant to be printed to the screen
struct PrintBuffer
{
    std::deque<char> received; //received is where we put messages that are received
    std::deque<char> sent;     //sent is where we put the message that was sent
};

//////

WConn& GetWConn();

PrintBuffer& GetPrintBuffer();

//////

class Timer
{
public:
	static char WaitForResponse(unsigned timeout);

private:
	static VOID CALLBACK TimerCallBack(HWND, UINT, UINT, DWORD);

	static bool timerCalledBack;
	static char response;
};

#endif // PROTOCOL_H