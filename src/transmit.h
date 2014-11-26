#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <windows.h>

DWORD WINAPI TransmitThread(LPVOID lpvThreadParm);

void Send();
void Transmit();
char SendData();
void ResetState();

#endif // TRANSMIT_H