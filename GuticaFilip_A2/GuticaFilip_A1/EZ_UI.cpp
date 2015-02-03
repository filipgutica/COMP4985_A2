#include "EZ_UI.h"

HWND CreateLabel(char *text, int x, int y, int w, int h, HWND hwnd)
{
	return CreateWindow(
		"static", 
		text,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        x, y, w, h,
        hwnd, 
		NULL,
        (HINSTANCE) GetWindowLong (hwnd, GWL_HINSTANCE), 
		NULL);
}

HWND CreateEditCtrl(char *text, int x, int y, int w, int h, HWND hwnd)
{
	return CreateWindow(
		"EDIT", 
		text, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_WANTRETURN, 
		x, y, w, h, 
		hwnd, 
		NULL, 
		(HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), 
		NULL);
}

HWND CreateBtn(char *text, int x, int y, int w, int h, HMENU id, HWND hwnd)
{
	return CreateWindow(
		"button",
		text,
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		x, y, w, h,
		hwnd, id,
		(HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), 
		NULL);
}

HWND CreateTextBox(int x, int y, int w, int h, HWND hwnd)
{
	return CreateWindowEx(
		0, 
		"EDIT",   // predefined class 
		NULL,         // no window title 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 
		x, y, w, h,  
		hwnd,         // parent window 
		(HMENU) IDM_RESULT,   // edit control ID 
		(HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), 
		NULL);        // pointer not needed 
}

HWND CreateRadioBtn(char *text, int x, int y, int w, int h, HMENU id, HWND hwnd)
{
	return  CreateWindow(
		"button",
		text,
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		x, y, w, h,
		hwnd, 
		id, 
		(HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), 
		NULL);
}

HWND CreateDropeDownList(int x, int y, int w, int h, HWND hwnd)
{
	return CreateWindow(WC_COMBOBOX,
             NULL,
             CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
             x, y, w, h,
             hwnd,
             NULL,
             (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
             NULL);
}
