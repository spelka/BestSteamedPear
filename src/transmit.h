#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <windows.h>

DWORD WINAPI TransmitThread(LPVOID);
VOID CALLBACK MyTimerProc(HWND, UINT, UINT, DWORD);

char SendChar(char sendChar, unsigned toDuration);
void SendPacket();

void Transmit();
void ResetState();

#endif // TRANSMIT_H