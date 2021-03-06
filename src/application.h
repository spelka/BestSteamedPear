#ifndef APPLICATION_H
#define APPLICATION_H

#include <Windows.h>
#include <string>

extern HWND hwnd;

struct GResources;
struct TextHolder;

enum txtholder_idx
{
	CHAT_LOG_RX, CHAT_LOG_TX, CURRENT_MSG, ALL
};

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

void PaintComponents();

void PrintToScreen(txtholder_idx whichHolder, std::string s, bool newlinebefore = false, bool newlineafter = true);
void PrintToScreen(txtholder_idx whichHolder, char c, bool newlinebefore = false, bool newlineafter = false);

void RedrawText(txtholder_idx whichHolder = ALL);

void ClearScreen(txtholder_idx whichHolder);

#endif // APPLICATION_H