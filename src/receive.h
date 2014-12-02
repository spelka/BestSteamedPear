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

DWORD WINAPI ReceiveThread(LPVOID lpvThreadParm);
char ReadChar(DWORD timeout);
bool FillRxBuffer();
bool ValidateData(GrapefruitPacket g);
#endif // LISTEN_H