#ifndef LISTEN_H
#define LISTEN_H

#include <deque>
#include <windows.h>
#include "protocol.h"

class SyncTracker
{
	public:
		static bool CheckSync(char syncbit);
		static void FlagForReset();
	private:
		static bool firstSync;
		static bool previousSync;
};

char ReadChar(DWORD timeout);
bool FillRxBuffer();
bool CheckForETX();
void ClearPrintBuffer();

#endif // LISTEN_H