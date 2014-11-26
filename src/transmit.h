#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <windows.h>

DWORD WINAPI TransmitThread(LPVOID);
VOID CALLBACK MyTimerProc(HWND, UINT, UINT, DWORD);

void Send();
void Transmit();
char SendData();
void ResetState();

#endif // TRANSMIT_H