#ifndef LISTEN_H
#define LISTEN_H

#include <deque>
#include <windows.h>
#include "protocol.h"

char ReadChar(DWORD timeout);
bool FillRxBuffer();
bool CheckForETX();
void ClearPrintBuffer();

#endif // LISTEN_H