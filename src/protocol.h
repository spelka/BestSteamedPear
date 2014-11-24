#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <vector>

const char ENQ = 0x05;
const char ACK = 0x06;
const char RVI = 0x17;
const char NAK = 0x21;

struct WConn
{
	std::vector<char> buffer_receive;
	std::vector<char> buffer_send;
};

#endif // PROTOCOL_H