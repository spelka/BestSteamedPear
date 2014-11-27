#ifndef LISTEN_H
#define LISTEN_H

#include <Windows.h>

void SendACK(HANDLE hComm);
bool ReceiveChar(char expectedChar);
char ReceiveChar();
void receivePacket(WConn* w);
void invalidData();
void packetValidator();
void timeOut1();
void validDataEOT();
void validDataETB();


#endif // LISTEN_H