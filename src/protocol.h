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

struct WConn
{
	HANDLE hComm;

	unsigned
		TO1,
		TO2,
		TO3,
		TO4
		;

	std::deque<char> buffer_receive;
	std::deque<char> buffer_send;
};

WConn& GetWConn();

class Timer
{
public:
	Timer();
	~Timer();

	char WaitForResponse();

private:
	VOID CALLBACK TimerCallBack(HWND, UINT, UINT, DWORD);

	bool timerCalledBack;
	char response;
};

DWORD WINAPI TransmitThread(LPVOID);

#endif // PROTOCOL_H