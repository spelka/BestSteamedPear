#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <windows.h>

void Transmit();

char SendChar(char charToSend, unsigned toDuration);
char SendPacket();

void TransmitMode();
void ResetState();

#endif // TRANSMIT_H