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

char Timer::WaitForResponse(unsigned timeout)
{
	SetTimer(NULL,					// handle to main window 
		42,							// timer identifier 
		timeout,					// timeout
		(TIMERPROC)TimerCallBack);	// timer callback

	while (!timerCalledBack)
	{
		response = ReceiveChar();

		// response received
		if (response != NUL) break;
	}

	timerCalledBack = false;

	return response;
}

VOID CALLBACK Timer::TimerCallBack(
	HWND hwnd,        // handle to window for timer messages 
	UINT message,     // WM_TIMER message 
	UINT idTimer,     // timer identifier 
	DWORD dwTime)     // current system time 
{
	timerCalledBack = true;
}