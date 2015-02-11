/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	EZ_UI.cpp	-	Wrapper functions to easily create UI elements.
--
--	PROGRAM:		Assignment2
--
--	FUNCTIONS:		CreateLabel
--					CreateEditCtrl
--					CreateBtn
--					CreateTextBox
--					CreateRadioBtn
--					CreateDropDownList
--
--	DATE:			February 9, 2015
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
--
--	NOTES:
--     
---------------------------------------------------------------------------------------*/
#include "EZ_UI.h"

/*------------------------------------------------------------------------------
--	FUNCTION: CreateLabel()
--
--	PURPOSE:		Wrapper function to create a label element.
--
--	PARAMETERS:
--		char *text	- Text to be displayed by the label
--		int x		- X position of the label
--      int y		- Y position of the label
--      int w		- Width of the label
--      int h		- Height of the lebel
--      HWNND h		- Parent Window
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
--	FUNCTION: CreateEditCtrl()
--
--	PURPOSE:		Wrapper function to create an edit control element.
--
--	PARAMETERS:
--		char *text	- Text to be displayed by the edit control
--		int x		- X position of the edit control
--      int y		- Y position of the edit control
--      int w		- Width of the edit control
--      int h		- Height of the edit control
--      HWNND h		- Parent Window
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
--	FUNCTION: CreateBtn()
--
--	PURPOSE:		Wrapper function to create a button element.
--
--	PARAMETERS:
--		char *text	- Text to be displayed by the button
--		int x		- X position of the button
--      int y		- Y position of the button
--      int w		- Width of the button
--      int h		- Height of the button
--		HMENU id	- ID of the button
--      HWNND h		- Parent Window
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
--	FUNCTION: CreateTextBox()
--
--	PURPOSE:		Wrapper function to create a text box element.
--
--	PARAMETERS:
--		char *text	- Text to be displayed by the text box
--		int x		- X position of the text box
--      int y		- Y position of the text box
--      int w		- Width of the text box
--      int h		- Height of the text box
--      HWNND h		- Parent Window
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
--	FUNCTION: CreateRadioBtn()
--
--	PURPOSE:		Wrapper function to create a radio button element.
--
--	PARAMETERS:
--		char *text	- Text to be displayed by the radio button
--		int x		- X position of the radio button
--      int y		- Y position of the radio button
--      int w		- Width of the radio button
--      int h		- Height of the radio button
--		HMENU id	- ID of the radio button
--      HWNND h		- Parent Window
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
--	FUNCTION: CreateDropDownList()
--
--	PURPOSE:		Wrapper function to create a drop down list element.
--
--	PARAMETERS:
--		int x		- X position of the drop down list
--      int y		- Y position of the drop down list
--      int w		- Width of the radio drop down list
--      int h		- Height of the drio down list
--      HWNND h		- Parent Window
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
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
