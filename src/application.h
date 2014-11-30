#ifndef APP_H
#define APP_H

#include <windows.h>

int WINAPI WinMain(
	HINSTANCE
	, HINSTANCE
	, LPSTR
	, int
	);

LRESULT CALLBACK WndProc(
	HWND
	, UINT
	, WPARAM
	, LPARAM
	);

#endif // APP_H