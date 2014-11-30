#include "application.h"

#include <stdio.h>
#include <vector>
#include <sstream>
#include "crc.h"

using namespace std;

//--->

HWND hwnd; // the window handle

//--->

struct GResources // Graphics Resources
{
	static HDC GetDC();

	static HFONT		hFont;
	static TEXTMETRIC	textMetric;
	static unsigned		textSizeX, textSizeY;
	static COLORREF		color_bk, color_txt, color_txt_bk;
};

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	GResources::GetDC
--
-- DATE: 		October 11, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Melvin Loho
--
-- PROGRAMMER: 	Melvin Loho
--
-- INTERFACE: 	HDC GResources::GetDC()
--
-- RETURNS: 	The handle to a device context.
--
-- NOTES:
-- Provides the handle to a device context that has been customized by the specifications found inside of the GResources struct.
----------------------------------------------------------------------------------------------------------------------*/
HDC GResources::GetDC()
{
	static HDC hdc;

	hdc = ::GetDC(hwnd); // get device context

	SelectObject(hdc, hFont); // set font
	GetTextMetrics(hdc, &textMetric); // get font/text metrics

	textSizeX = textMetric.tmAveCharWidth + textMetric.tmOverhang;
	textSizeY = textMetric.tmHeight + textMetric.tmExternalLeading;

	SetBkColor(hdc, color_txt_bk);
	SetTextColor(hdc, color_txt);

	return hdc;
}

HFONT		GResources::hFont;

TEXTMETRIC	GResources::textMetric;

unsigned	GResources::textSizeX;
unsigned	GResources::textSizeY;

COLORREF	GResources::color_bk;
COLORREF	GResources::color_txt;
COLORREF	GResources::color_txt_bk;

//--->

struct TextHolder
{
	static vector<TextHolder> txtHolders;

	string txtTitle;				// the title
	vector<string> txtBuffer;		// the text buffer
	RECT txtRect;					// the text bounding box
	COLORREF color_txt;				// the text color
};

vector<TextHolder> TextHolder::txtHolders; // the container for all of the text holders

//--->

int WINAPI WinMain(HINSTANCE hInst    //_In_  HINSTANCE hInstance,
	, HINSTANCE hprevInstance         //_In_  HINSTANCE hPrevInstance,
	, LPSTR lspszCmdParam             //_In_  LPSTR lpCmdLine,
	, int nCmdShow)                   //_In_  int nCmdShow
{
	/** crc test
	unsigned char test[] = "123456789";
	stringstream ss;

	ss << "The check value for the " << CRC_NAME << " standard is 0x" << CHECK_VALUE << endl;
	ss << "The crcSlow() of \"123456789\" is 0x" << crcSlow(test, strlen(reinterpret_cast<char*>(test))) << endl;
	crcInit();
	ss << "The crcFast() of \"123456789\" is 0x" << crcFast(test, strlen(reinterpret_cast<char*>(test))) << endl;
	OutputDebugString( ss.str().c_str() );
	*/

	MSG msg;
	WNDCLASSEX Wcl;
	LOGFONT logFont;
	const char Name[] = "Best Steamed Pear";

	// set colors
	GResources::color_bk = RGB(83, 83, 83);
	GResources::color_txt = RGB(242, 242, 242);
	GResources::color_txt_bk = RGB(38, 38, 38);

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = CreateSolidBrush(GResources::color_bk);
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = "MYMENU"; // The menu Class
	Wcl.cbClsExtra = 0;			 // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInst, NULL);

	// set the font properties
	memset(&logFont, 0, sizeof(logFont));
	logFont.lfHeight = 18;
	logFont.lfWeight = FW_DONTCARE;
	strcpy_s(logFont.lfFaceName, "Consolas");
	GResources::hFont = CreateFontIndirect(&logFont);

	// initialize text holders
	TextHolder::txtHolders.push_back(TextHolder()); // CHAT_LOG
	TextHolder::txtHolders.push_back(TextHolder()); // CURRENT_MSG

	TextHolder::txtHolders[CHAT_LOG].color_txt = RGB(225, 225, 225);
	TextHolder::txtHolders[CURRENT_MSG].color_txt = RGB(0, 200, 0);

	// initialize the text buffer
	ClearScreen(ALL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd
	, UINT msg
	, WPARAM wParam
	, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	PaintComponents
--
-- DATE: 		October 11, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Cody Gray, Melvin Loho
--
-- PROGRAMMER: 	Melvin Loho
--
-- INTERFACE: 	void PaintComponents()
--
-- RETURNS: 	void.
--
-- NOTES:
-- Originally from a stackoverflow answer: http://stackoverflow.com/questions/16159127/win32-how-to-draw-a-rectangle-around-a-text-string
-- Paints all of the GUI components, as well as some titles to accompany those components.
-- Does all of the dimension calculations in order to paint the correct sized components.
----------------------------------------------------------------------------------------------------------------------*/
void PaintComponents()
{
	static HBRUSH color_bk_rects;
	static RECT rcWindow;
	static RECT rc_chatlog, rc_currmsg;
	static int inflateAmount = -8;
	static char breakline = '—';

	// Set up the device context for drawing
	HDC hDC = GResources::GetDC();
	HPEN hpenOld = static_cast<HPEN>(SelectObject(hDC, GetStockObject(DC_PEN)));
	HBRUSH hbrushOld = static_cast<HBRUSH>(SelectObject(hDC, GetStockObject(NULL_BRUSH)));

	// Calculate the dimensions of the rectangles
	GetClientRect(hwnd, &rcWindow);
	InflateRect(&rcWindow, inflateAmount, inflateAmount);

	rc_chatlog = rc_currmsg = rcWindow;

	rc_chatlog.right *= (1.0 / 3.5);
	rc_chatlog.bottom *= (0.8);

	rc_currmsg.top = rc_chatlog.bottom;

	// Add spacing between the rectangles
	InflateRect(&rc_currmsg, inflateAmount, inflateAmount);
	InflateRect(&rc_chatlog, inflateAmount, inflateAmount);

	// Draw borders around these rectangles
	color_bk_rects = CreateSolidBrush(GResources::color_txt_bk);
	SetDCPenColor(hDC, RGB(60, 60, 60));

	// chat log
	FillRect(hDC, &rc_chatlog, color_bk_rects);
	Rectangle(hDC, rc_chatlog.left, rc_chatlog.top, rc_chatlog.right, rc_chatlog.bottom);

	// current message
	FillRect(hDC, &rc_currmsg, color_bk_rects);
	Rectangle(hDC, rc_currmsg.left, rc_currmsg.top, rc_currmsg.right, rc_currmsg.bottom);

	// Add padding for the text
	InflateRect(&rc_currmsg, inflateAmount, inflateAmount);
	InflateRect(&rc_chatlog, inflateAmount, inflateAmount);

	// Set txt holders's new rects
	TextHolder::txtHolders[CHAT_LOG].txtRect = rc_chatlog;
	TextHolder::txtHolders[CURRENT_MSG].txtRect = rc_currmsg;

	// Set and draw titles
	TextHolder::txtHolders[CHAT_LOG].
		txtTitle = std::string("Chat Log\n")
		.append(std::string((rc_chatlog.right - rc_chatlog.left) / GResources::textSizeX, breakline));

	TextHolder::txtHolders[CURRENT_MSG].
		txtTitle = std::string("Enter message\n")
		.append(std::string((rc_currmsg.right - rc_currmsg.left) / GResources::textSizeX, breakline));

	DrawText(hDC, TEXT(TextHolder::txtHolders[CHAT_LOG].
		txtTitle.c_str()), -1, &rc_chatlog, DT_CENTER);
	DrawText(hDC, TEXT(TextHolder::txtHolders[CURRENT_MSG].
		txtTitle.c_str()), -1, &rc_currmsg, DT_CENTER);

	// Clean up after ourselves
	SelectObject(hDC, hpenOld);
	SelectObject(hDC, hbrushOld);
	ReleaseDC(hwnd, hDC); // Release device context
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	PrintToScreen
--
-- DATE: 		October 11, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Melvin Loho
--
-- PROGRAMMER: 	Melvin Loho
--
-- INTERFACE: 	void PrintToScreen(txtholder_idx whichHolder, std::string s, bool newlinebefore = false, bool newlineafter = true)
--					txtholder_idx whichHolder	: the text holder to append the string to.
--					std::string s				: The string to display and append.
--					bool newlinebefore			: True if there should there be a newline before the string, false otherwise.
--					bool newlineafter			: True if there should there be a newline after the string, false otherwise.
--
-- RETURNS: 	void.
--
-- NOTES:
-- Prints a string to the screen. Also appends the string into the specified text buffer.
----------------------------------------------------------------------------------------------------------------------*/
void PrintToScreen(txtholder_idx whichHolder, std::string s, bool newlinebefore, bool newlineafter)
{
	HDC hdc = GResources::GetDC();

	if (newlinebefore)
	{
		TextHolder::txtHolders[whichHolder].txtBuffer.push_back("\n");
	}

	TextHolder::txtHolders[whichHolder].txtBuffer.push_back(s);

	if (newlineafter)
	{
		TextHolder::txtHolders[whichHolder].txtBuffer.push_back("\n");
	}

	ReleaseDC(hwnd, hdc); // Release device context

	RedrawText(whichHolder);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	RedrawText
--
-- DATE: 		October 11, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Melvin Loho
--
-- PROGRAMMER: 	Melvin Loho
--
-- INTERFACE: 	void RedrawText(txtholder_idx whichHolder = ALL)
--					txtholder_idx whichHolder: the text holder to redraw the strings from
--
-- RETURNS: 	void.
--
-- NOTES:
-- Redraws the specified text buffer onto the screen.
----------------------------------------------------------------------------------------------------------------------*/
void RedrawText(txtholder_idx whichHolder)
{
	std::string currTxt;

	HDC hdc = GResources::GetDC();

	if (whichHolder == ALL)
	{
		// loop through every text holder
		for (unsigned t = 0; t < TextHolder::txtHolders.size(); ++t)
		{
			currTxt = "";

			// append each string into the current local text buffer
			for (unsigned i = 0; i < TextHolder::txtHolders[t].txtBuffer.size(); ++i)
			{
				currTxt += TextHolder::txtHolders[t].txtBuffer[i];
			}

			// draw final string
			SetTextColor(hdc, TextHolder::txtHolders[t].color_txt);
			DrawText(hdc, currTxt.c_str(), -1, &TextHolder::txtHolders[t].txtRect, DT_LEFT);
		}
	}
	else
	{
		currTxt = "";

		// append each string into the current local text buffer
		for (unsigned i = 0; i < TextHolder::txtHolders[whichHolder].txtBuffer.size(); ++i)
		{
			currTxt += TextHolder::txtHolders[whichHolder].txtBuffer[i];
		}

		// draw final string
		SetTextColor(hdc, TextHolder::txtHolders[whichHolder].color_txt);
		DrawText(hdc, currTxt.c_str(), -1, &TextHolder::txtHolders[whichHolder].txtRect, DT_LEFT);
	}

	ReleaseDC(hwnd, hdc); // Release device context
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	ClearScreen
--
-- DATE: 		October 11, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Melvin Loho
--
-- PROGRAMMER: 	Melvin Loho
--
-- INTERFACE: 	void ClearScreen(txtholder_idx whichHolder)
--					txtholder_idx whichHolder: the text holder to clear
--
-- RETURNS: 	void.
--
-- NOTES:
-- Clears the specified text buffer and refreshes the screen which the text buffer occupies.
----------------------------------------------------------------------------------------------------------------------*/
void ClearScreen(txtholder_idx whichHolder)
{
	static HBRUSH color_bk_rects;

	HDC hdc = GResources::GetDC();

	color_bk_rects = CreateSolidBrush(GResources::color_txt_bk);

	if (whichHolder == ALL)
	{
		for (unsigned t = 0; t < TextHolder::txtHolders.size(); ++t)
		{
			// reset buffer
			TextHolder::txtHolders[t].txtBuffer.clear();
			TextHolder::txtHolders[t].txtBuffer.push_back("\n\n");

			// refresh the area
			FillRect(hdc, &TextHolder::txtHolders[t].txtRect, color_bk_rects);
			DrawText(hdc, TEXT(TextHolder::txtHolders[t].
				txtTitle.c_str()), -1, &TextHolder::txtHolders[t].txtRect, DT_CENTER);
		}
	}
	else
	{
		// reset buffer
		TextHolder::txtHolders[whichHolder].txtBuffer.clear();
		TextHolder::txtHolders[whichHolder].txtBuffer.push_back("\n\n");

		// refresh the area
		FillRect(hdc, &TextHolder::txtHolders[whichHolder].txtRect, color_bk_rects);
		DrawText(hdc, TEXT(TextHolder::txtHolders[whichHolder].
			txtTitle.c_str()), -1, &TextHolder::txtHolders[whichHolder].txtRect, DT_CENTER);
	}

	ReleaseDC(hwnd, hdc); // Release device context
}