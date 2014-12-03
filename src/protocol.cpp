/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		protocol.cpp
--
-- PROGRAM:			BestSteamPear
--
-- FUNCTIONS:		WConn& GetWConn()
					PrintBuffer& GetPrintBuffer()
					char Timer::WaitForResponse(unsigned timeout)

-- DATE: 			November 29, 2014
--
-- REVISIONS: 		NONE
--
-- DESIGNER: 		Melvin Loho, Alex Lam, Sebastian Pelka, Georgi Hristov
--
-- PROGRAMMER: 		Melvin Loho
--
-- NOTES:
-- This is the code that takes care of all the trasmitting that is done in this program.
----------------------------------------------------------------------------------------------------------------------*/

#include "protocol.h"

#include "receive.h"
#include "transmit.h"
#include "application.h"
#include <stdlib.h>

using namespace std;

//////

char Timer::response = NUL;
bool Timer::timerCalledBack = false;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTIONS:		Timer::WaitForResponse(unsigned)
--
-- DATE: 			November 29, 2014
--
-- REVISIONS: 		NONE
--
-- DESIGNER: 		Melvin Loho, Alex Lam, Sebastian Pelka, Georgi Hristov
--
-- PROGRAMMER: 		Melvin Loho
--
-- NOTES:
-- This creates a timer that attempts to receive a char from the received buffer until timeout is exceeded.
----------------------------------------------------------------------------------------------------------------------*/
char Timer::WaitFor(unsigned timeout)
{
	SetTimer(NULL,					// handle to main window 
		42,							// timer identifier 
		timeout,					// timeout
		(TIMERPROC)TimerCallBack);	// timer callback

	while (!timerCalledBack);

	timerCalledBack = false;

	return response;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTIONS:		Timer::TimerCallBack
--
-- DATE: 			November 29, 2014
--
-- REVISIONS: 		NONE
--
-- DESIGNER: 		Melvin Loho, Alex Lam, Sebastian Pelka, Georgi Hristov
--
-- PROGRAMMER: 		Melvin Loho
--
-- NOTES:
-- Lets the other classes know that timer has finished, which usually means timeout has occured.
----------------------------------------------------------------------------------------------------------------------*/
VOID CALLBACK Timer::TimerCallBack(
	HWND hwnd,        // handle to window for timer messages 
	UINT message,     // WM_TIMER message 
	UINT idTimer,     // timer identifier 
	DWORD dwTime)     // current system time 
{
	timerCalledBack = true;


	MessageBox(NULL, "", "TIMERCALLEDBACK", MB_OK);
}

//////

HANDLE hReceiveThread;				// the reading thread handle
DWORD idReceiveThread;				// the reading thread handle identification

HANDLE hTransmitThread;				// the transmitting thread handle
DWORD idTransmitThread;				// the transmitting thread handle identification

WConn& GetWConn()
{
	static WConn wConn = { 0 };
	return wConn;
}

bool Configure(LPCSTR lpszCommName)
{
	WConn& wConn = GetWConn();
	wConn.lpszCommName = lpszCommName;

	wConn.cc.dwSize = sizeof(COMMCONFIG);
	GetCommConfig(wConn.hComm, &wConn.cc, &wConn.cc.dwSize);
	if (!CommConfigDialog(wConn.lpszCommName, hwnd, &wConn.cc))
		return false;
	SetCommState(wConn.hComm, &wConn.cc.dcb);

	wConn.TO3 = 1200.0 * 8.0 / wConn.cc.dcb.BaudRate * 1000.0;
	wConn.TO1 = wConn.TO3 * MAX_MISS * 1000.0;
	wConn.TO2 = 5.0 * 8.0 / wConn.cc.dcb.BaudRate * 1000.0;
	wConn.TO4 = (rand() % 4 + 1) * 8.0 / wConn.cc.dcb.BaudRate * 1000.0;

	return true;
}

bool Connect()
{
	WConn& wConn = GetWConn();

	wConn.isConnected = false;

	if ((wConn.hComm = CreateFile(wConn.lpszCommName, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL))
		== INVALID_HANDLE_VALUE)
	{
		MessageBox(hwnd, std::string("Error opening COM port: ").append(wConn.lpszCommName).c_str(), "", MB_OK);
		return false;
	}

	if (!(hReceiveThread = CreateThread(NULL, 0, ReceiveThread, (LPVOID)NULL, 0, &idReceiveThread)))
	{
		MessageBox(hwnd, "Error creating comm port reading thread", "", MB_OK);
		return false;
	}

	if (!(hTransmitThread = CreateThread(NULL, 0, TransmitThread, (LPVOID)NULL, 0, &idTransmitThread)))
	{
		MessageBox(hwnd, "Error creating comm port transmitting thread", "", MB_OK);
		return false;
	}

	wConn.isConnected = true;

	return true;
}

bool Disconnect()
{
	WConn& wConn = GetWConn();

	wConn.isConnected = false;
	return (CloseHandle(hReceiveThread) && CloseHandle(hTransmitThread) && CloseHandle(wConn.hComm));
}