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
		static char previousSync;
};

char ReadChar(DWORD timeout);
bool FillRxBuffer();
bool ValidateData();
void TrimPacket();
void ClearPrintBuffer();

#endif // LISTEN_H