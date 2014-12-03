#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <windows.h>
#include <deque>

#define PACKET_TOTAL_SIZE 1024
#define PACKET_DATA_SIZE 1018
#define PACKET_CRC_SIZE 4

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

const unsigned MAX_SEND = 10;
const unsigned MAX_MISS = 3;

//////

struct GrapefruitPacket
{
	unsigned char ctrl;
	unsigned char sync;
	unsigned char data[PACKET_DATA_SIZE];
	unsigned char crc[PACKET_CRC_SIZE];
};

//////

struct WConn
{
	enum status
	{
		DEAD, IDLE, RECEIVING, TRANSMITTING
	};

	HANDLE hComm;
	COMMCONFIG cc;
	LPCSTR lpszCommName;

	HANDLE hReceiveThread;				// the reading thread handle
	DWORD idReceiveThread;				// the reading thread handle identification

	HANDLE hTransmitThread;				// the transmitting thread handle
	DWORD idTransmitThread;				// the transmitting thread handle identification

	std::deque<char> buffer_rx_ctrl;
	std::deque<GrapefruitPacket> buffer_rx_packet;
	std::deque<GrapefruitPacket> buffer_tx;

	DWORD TO1, TO2, TO3, TO4;

	status status;

	bool canTransmit;
    bool rvi;
	bool synFlip;
};

//////

WConn& GetWConn();

bool Configure(LPCSTR lpszCommName);
bool Connect();
bool Disconnect();

#endif // PROTOCOL_H