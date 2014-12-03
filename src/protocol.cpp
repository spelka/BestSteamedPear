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

WConn& GetWConn()
{
	static WConn wConn;
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

	wConn.status = WConn::IDLE;

	if ((wConn.hComm = CreateFile(wConn.lpszCommName, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, NULL, NULL))
		== INVALID_HANDLE_VALUE)
	{
		MessageBox(hwnd, std::string("Error opening COM port: ").append(wConn.lpszCommName).c_str(), "", MB_OK);
		return false;
	}

	if (!(wConn.hReceiveThread = CreateThread(NULL, 0, ReceiveThread, (LPVOID)NULL, 0, &wConn.idReceiveThread)))
	{
		MessageBox(hwnd, "Error creating comm port reading thread", "", MB_OK);
		return false;
	}

	if (!(wConn.hTransmitThread = CreateThread(NULL, 0, TransmitThread, (LPVOID)NULL, 0, &wConn.idTransmitThread)))
	{
		MessageBox(hwnd, "Error creating comm port transmitting thread", "", MB_OK);
		return false;
	}

	return true;
}

bool Disconnect()
{
	WConn& wConn = GetWConn();

	wConn.status = WConn::DEAD;
	CancelSynchronousIo(wConn.hReceiveThread);
	CancelSynchronousIo(wConn.hTransmitThread);
	return (CloseHandle(wConn.hReceiveThread) && CloseHandle(wConn.hTransmitThread) && CloseHandle(wConn.hComm));
}