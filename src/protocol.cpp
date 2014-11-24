#include "protocol.h"

WConn& GetWConn()
{
	static WConn wConn;
	return wConn;
}