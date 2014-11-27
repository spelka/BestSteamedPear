#ifndef LISTEN_H
#define LISTEN_H

void SendACK(HANDLE hComm);
bool ReceiveChar(char expectedChar);
char ReceiveChar();
void receivePacket(WConn* w);


#endif // LISTEN_H