#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <windows.h>

char WaitForResponse();

char SendChar(char charToSend, unsigned toDuration);
char SendPacket();

void Transmit();
void ResetState();

#endif // TRANSMIT_H