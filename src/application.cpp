#include "application.h"

#include <stdio.h>
#include <vector>
#include <sstream>
#include "crc.h"
#include "resource.h"
#include "protocol.h"

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

	string txtTitle;				// title
	vector<string> txtBuffer;		// text buffer
	RECT txtRect;					// text bounding box
	COLORREF color_txt;				// text color
	UINT format;			// text format
};

vector<TextHolder> TextHolder::txtHolders; // the container for all of the text holders

//--->

const char Name[] = "Best Steamed Pear";

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
	TextHolder::txtHolders.push_back(TextHolder()); // CHAT_LOG_RX
	TextHolder::txtHolders.push_back(TextHolder()); // CHAT_LOG_TX
	TextHolder::txtHolders.push_back(TextHolder()); // CURRENT_MSG

	TextHolder::txtHolders[CHAT_LOG_RX].color_txt = RGB(162, 196, 253);
	TextHolder::txtHolders[CHAT_LOG_TX].color_txt = RGB(93, 254, 142);
	TextHolder::txtHolders[CURRENT_MSG].color_txt = RGB(255, 225, 225);

	TextHolder::txtHolders[CHAT_LOG_RX].format = DT_LEFT | DT_EXPANDTABS;
	TextHolder::txtHolders[CHAT_LOG_TX].format = DT_RIGHT | DT_EXPANDTABS;
	TextHolder::txtHolders[CURRENT_MSG].format = DT_LEFT | DT_EXPANDTABS;

	// initialize the text buffer
	ClearScreen(ALL);

	// initialize comm port settings
	GetWConn().lpszCommName = "(UNSET)";

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 	WndProc
--
-- DATE: 		October 11, 2014
--
-- REVISIONS: 	NONE
--
-- DESIGNER: 	Aman Abdulla, Melvin Loho
--
-- PROGRAMMER: 	Melvin Loho
--
-- INTERFACE: 	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)
--					HWND	: A handle to the window.
--					UINT	: The message.
--					WPARAM	: Additional message information.
--					LPARAM	: Additional message information.
--
-- RETURNS: 	The return value is the result of the message processing and depends on the message sent.
--
-- NOTES:
-- Originally from Aman's winmenu4 program.
-- Handles all of the events (messages) sent to the window.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	static PAINTSTRUCT paintstruct;

	static HMENU mymenu = GetMenu(hwnd);
	static MENUITEMINFO mii_connect = { sizeof(MENUITEMINFO), MIIM_STRING, MFT_STRING };

	static MINMAXINFO* minmaxInfo;

	static std::string helpMsg =
		std::string()
		.append("[- ").append(Name).append(" -]")
		.append("\n")
		.append("\nby:")
		.append("\n+ Melvin Loho | A00885598")
		.append("\n+ Alex Lam | A00880208")
		.append("\n+ Georgi Hristov | A00795026")
		.append("\n+ Orange Hat Guy | A00888888")
		.append("\n")
		.append("\n[Menu Items]")
		.append("\n> Connect/Disconnect - Connect to / disconnects from a SkyeTek reader.")
		.append("\n> Clear Screen - Clears the contents of the screen.")
		.append("\n> Help - Brings up this menu!")
		.append("\n> Exit to CMD - Closes the connection and the window, while opening a command window.")
		;
    
    static int newLines = 1;

	string currMsg;

	switch (Message)
	{
	case WM_COMMAND:			// menu items

		switch (LOWORD(wParam))
		{
		case ID_CONFIGURE:
			Configure("COM1");
			break;

		case ID_CONNECT:
			if (!GetWConn().isConnected)
			{
				if (Connect())
				{
					ClearScreen(ALL);
					//PrintToScreen(LOG, "> Connected to " + std::string("a SkyeTek reader") + "");
				}
				else
				{
					Disconnect();
					//PrintToScreen(LOG, "> Failed to connect to " + std::string("a SkyeTek reader") + "");
				}
			}
			else
			{
				Disconnect();
			}
			// refresh the menu with the new information
			if (GetWConn().isConnected)	mii_connect.dwTypeData = "Disconnect";
			else							mii_connect.dwTypeData = "Connect";
			SetMenuItemInfo(mymenu, ID_CONNECT, FALSE, &mii_connect);
			DrawMenuBar(hwnd);
			break;

		case ID_CLS:
			ClearScreen(CHAT_LOG_RX);
			ClearScreen(CHAT_LOG_TX);
			break;

		case ID_HELP:
			MessageBox(hwnd, helpMsg.c_str(), "Help", MB_OK); // display help
			break;

		case ID_EXIT:
			if (GetWConn().isConnected) Disconnect();
			PostQuitMessage(0);
			system("cmd"); // open a command prompt
			break;
		}
		break;

	case WM_CHAR:				// Process keystroke
		if (GetWConn().isConnected)
		{
			if (wParam == VK_ESCAPE)
			{
				Disconnect();
				// refresh the menu with the new information
				mii_connect.dwTypeData = "Connect";
				SetMenuItemInfo(mymenu, ID_CONNECT, FALSE, &mii_connect);
				DrawMenuBar(hwnd);
			}
		}

		switch (wParam)
		{
		case VK_RETURN:
			currMsg = TextHolder::txtHolders[CURRENT_MSG].txtBuffer.back();

			if (currMsg.empty()) break;

			PrintToScreen(CHAT_LOG_TX, currMsg);
            for( ; newLines > 0; --newLines )
			    PrintToScreen(CHAT_LOG_RX, "");
            ++newLines;

			ClearScreen(CURRENT_MSG);
			break;

		case VK_ESCAPE:
			currMsg = TextHolder::txtHolders[CURRENT_MSG].txtBuffer.back();

			if (currMsg.empty()) break;

			PrintToScreen(CHAT_LOG_RX, currMsg);
            for( ; newLines > 0; --newLines )
			    PrintToScreen(CHAT_LOG_TX, "");
            ++newLines;

			ClearScreen(CURRENT_MSG);
			break;

        case 0x0A:
            ++newLines;
			PrintToScreen(CURRENT_MSG, wParam);
            break;

		default:
			PrintToScreen(CURRENT_MSG, wParam);
            //if( TextHolder::txtHolders[CURRENT_MSG].txtBuffer.back().size() == 0 )
                //++newLines;
            break;
		}
		break;

	case WM_PAINT:				// Process a repaint message
		BeginPaint(hwnd, &paintstruct);
		PaintComponents();
		RedrawText();
		EndPaint(hwnd, &paintstruct);
		break;

	case WM_GETMINMAXINFO:		// Give app the min/max window sizes
		minmaxInfo = (MINMAXINFO*)lParam;
		minmaxInfo->ptMinTrackSize.x = 600;
		minmaxInfo->ptMinTrackSize.y = 600;
		break;

	case WM_DESTROY:			// Terminate program
		if (GetWConn().isConnected) Disconnect();

		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
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

	rc_chatlog.bottom *= (0.7);

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
	TextHolder::txtHolders[CHAT_LOG_RX].txtRect = rc_chatlog;
	TextHolder::txtHolders[CHAT_LOG_TX].txtRect = rc_chatlog;
	TextHolder::txtHolders[CURRENT_MSG].txtRect = rc_currmsg;

	// Set and draw titles
	TextHolder::txtHolders[CHAT_LOG_RX].
		txtTitle = std::string("[= Chat Log =]\n")
		.append(std::string((rc_chatlog.right - rc_chatlog.left) / GResources::textSizeX, breakline));

	TextHolder::txtHolders[CHAT_LOG_TX].
		txtTitle = std::string("[= Chat Log =]\n")
		.append(std::string((rc_chatlog.right - rc_chatlog.left) / GResources::textSizeX, breakline));

	TextHolder::txtHolders[CURRENT_MSG].
		txtTitle = std::string("[= Enter Message =]\n")
		.append(std::string((rc_currmsg.right - rc_currmsg.left) / GResources::textSizeX, breakline));

	DrawText(hDC, TEXT(TextHolder::txtHolders[CHAT_LOG_RX].
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

void PrintToScreen(txtholder_idx whichHolder, char c)
{
	HDC hdc = GResources::GetDC();

	switch (c)
	{
	case VK_BACK:
		if (TextHolder::txtHolders[whichHolder].txtBuffer.back().empty()) break;

		TextHolder::txtHolders[whichHolder].txtBuffer.back().pop_back();
		TextHolder::txtHolders[whichHolder].txtBuffer.back().push_back(' ');
		RedrawText(whichHolder);
		TextHolder::txtHolders[whichHolder].txtBuffer.back().pop_back();
		break;

	case VK_TAB:
		TextHolder::txtHolders[whichHolder].txtBuffer.back().push_back('\t');
		break;

	default:
		TextHolder::txtHolders[whichHolder].txtBuffer.back().push_back(c);
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
			DrawText(hdc, const_cast<char *>(currTxt.c_str()), -1, &TextHolder::txtHolders[t].txtRect, TextHolder::txtHolders[t].format);
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
		DrawText(hdc, const_cast<char *>(currTxt.c_str()), -1, &TextHolder::txtHolders[whichHolder].txtRect, TextHolder::txtHolders[whichHolder].format);
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
			TextHolder::txtHolders[t].txtBuffer.push_back("");

			// refresh the area
			FillRect(hdc, &TextHolder::txtHolders[t].txtRect, color_bk_rects);
			DrawText(hdc, const_cast<char *>(TextHolder::txtHolders[t].
				txtTitle.c_str()), -1, &TextHolder::txtHolders[t].txtRect, DT_CENTER);
		}
	}
	else
	{
		// reset buffer
		TextHolder::txtHolders[whichHolder].txtBuffer.clear();
		TextHolder::txtHolders[whichHolder].txtBuffer.push_back("\n\n");
		TextHolder::txtHolders[whichHolder].txtBuffer.push_back("");

		// refresh the area
		FillRect(hdc, &TextHolder::txtHolders[whichHolder].txtRect, color_bk_rects);
		DrawText(hdc, const_cast<char *>(TextHolder::txtHolders[whichHolder].
			txtTitle.c_str()), -1, &TextHolder::txtHolders[whichHolder].txtRect, DT_CENTER);
	}

	ReleaseDC(hwnd, hdc); // Release device context
}