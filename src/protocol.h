#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <windows.h>
#include <vector>

const char NUL = 0x00;
const char ETX = 0x03;
const char ENQ = 0x05;
const char ACK = 0x06;
const char RVI = 0x17;
const char NAK = 0x21;
const char PAD = 0x54;

const unsigned PACKET_SIZE = 1024; //packet size in bytes
const unsigned PACKET_DATA_SIZE = 1018; //packet size in bytes

struct WConn
{
	HANDLE hComm;

	unsigned
		TO1,
		TO2,
		TO3,
		TO4
		;

	std::vector<char> buffer_receive;
	std::vector<char> buffer_send;
};

WConn& GetWConn();

#endif // PROTOCOL_H