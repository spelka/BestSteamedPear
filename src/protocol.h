#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <windows.h>
#include <deque>

/*
const char NUL = 0x00;
const char ETX = 0x03;
const char EOT = 0x04;
const char ENQ = 0x05;
const char ACK = 0x06;
const char RVI = 0x17;
const char NAK = 0x21;
const char ETB = 0x23;
const char PAD = 0x54;
*/

const char NUL = 'U';
const char ETX = 'T';
const char EOT = 'O';
const char ENQ = 'E';
const char ACK = 'A';
const char RVI = 'R';
const char NAK = 'N';
const char ETB = 'B';
const char PAD = 'P';

const unsigned PACKET_SIZE = 1024; //packet size in bytes
const unsigned PACKET_DATA_SIZE = 1018; //packet size in bytes

const unsigned MAX_SEND = 10;
const unsigned MAX_MISS = 3;

//////

struct WConn
{
	HANDLE hComm;
	LPCSTR lpszCommName;

	std::deque<char> buffer_receive;
	std::deque<char> buffer_send;

	unsigned
		TO1,
		TO2,
		TO3,
		TO4
		;

	bool isConnected;
	bool canTransmit;
    bool rvi;
	bool synFlip;
};

//////

class Timer
{
public:
	static char WaitFor(unsigned timeout);

private:
	static VOID CALLBACK TimerCallBack(HWND, UINT, UINT, DWORD);

	static bool timerCalledBack;
	static char response;
};

//////

WConn& GetWConn();

bool Configure(LPCSTR lpszCommName);
bool Connect();
bool Disconnect();

#endif // PROTOCOL_H