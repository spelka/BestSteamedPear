#include "application.h"

#include <commctrl.h>
#include "crc.h"
#include <stdio.h>
#include <string>
#include <sstream>

char Name[] = "Best Steamed Pear";

int WINAPI WinMain(HINSTANCE hInst    //_In_  HINSTANCE hInstance,
	, HINSTANCE hprevInstance         //_In_  HINSTANCE hPrevInstance,
	, LPSTR lspszCmdParam             //_In_  LPSTR lpCmdLine,
	, int nCmdShow)                   //_In_  int nCmdShow
{
    //* crc test
    unsigned char  test[] = "123456789";
    std::stringstream ss;

    ss << "The check value for the " << CRC_NAME << " standard is 0x" << CHECK_VALUE << std::endl;
    ss << "The crcSlow() of \"123456789\" is 0x" << crcSlow(test, strlen(reinterpret_cast<char*>(test))) << std::endl;
	crcInit();
	ss << "The crcFast() of \"123456789\" is 0x" << crcFast(test, strlen(reinterpret_cast<char*>(test))) << std::endl;
    OutputDebugString( ss.str().c_str() );
	
    //*/
	HWND hWnd;
	HWND hStatus;
	MSG Msg;
	WNDCLASSEX windowClass;

	//Set up window class
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hIconSm = NULL;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = hInst;
	windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	windowClass.lpszClassName = Name;

	windowClass.lpszMenuName = TEXT("MYMENU");
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;

	if (!RegisterClassEx(&windowClass))
		return 0;

	hWnd = CreateWindow(Name   //_In_opt_  LPCTSTR lpClassName,
		, Name                 //_In_opt_  LPCTSTR lpWindowName,
		, WS_OVERLAPPEDWINDOW  //_In_      DWORD dwStyle,
		, 10                   //_In_      int x,
		, 10                   //_In_      int y,
		, 800                  //_In_      int nWidth,
		, 400                  //_In_      int nHeight,
		, NULL                 //_In_opt_  HWND hWndParent,
		, NULL                 //_In_opt_  HMENU hMenu,
		, hInst                //_In_opt_  HINSTANCE hInstance,
		, NULL);               //_In_opt_  LPVOID lpParam

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd
	, UINT msg
	, WPARAM wParam
	, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}