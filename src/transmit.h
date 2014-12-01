#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <windows.h>

DWORD WINAPI TransmitThread(LPVOID lpvThreadParm);

bool SendChar(char charToSend, unsigned toDuration);
bool SendPacket();

void Transmit();
void ResetState();

#endif // TRANSMIT_H