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

using namespace std;

WConn& GetWConn()
{
	static WConn wConn;
	return wConn;
}

//////

PrintBuffer& GetPrintBuffer()
{
    static PrintBuffer pBuff;
    return pBuff;
}

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
}