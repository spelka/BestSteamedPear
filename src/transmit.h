#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <Windows.h>

DWORD WINAPI TransmitThread(LPVOID lpvThreadParm);

void Send();

void Transmit();

void SendData();

void resetState();

VOID CALLBACK MyTimerProc(
	HWND hwnd,        // handle to window for timer messages 
	UINT message,     // WM_TIMER message 
	UINT idTimer,     // timer identifier 
	DWORD dwTime);     // current system time

#endif // TRANSMIT_H