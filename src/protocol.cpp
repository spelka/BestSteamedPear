#include "protocol.h"
#include "receive.h"

WConn& GetWConn()
{
	static WConn wConn;
	return wConn;
}

Timer::Timer()
	:timerCalledBack(false)
	,response(NUL)
{}
Timer::~Timer()
{}

char Timer::WaitForResponse()
{
	while (!timerCalledBack)
	{
		response = ReceiveChar();

		// response received
		if (response != NUL) break;
	}

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