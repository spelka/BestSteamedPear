#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <windows.h>
#include <string>

DWORD WINAPI TransmitThread(LPVOID lpvThreadParm);

void Packetize(std::string s);

bool SendChar(char charToSend);
bool SendPacket();

void Transmit();
void ResetState();

#endif // TRANSMIT_H