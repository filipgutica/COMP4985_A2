/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	Application.h - Header file for Application.cpp
--									Contains function headers, and Necesarry 
--									global declarations such as the handles to the
--									UI elements.
--
--	PROGRAM:		Network_Resolver
--
--
--	DATE:			January 14, 2015
--
--	REVISIONS:		(Date and Description)
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
--
--	NOTES:
--     
---------------------------------------------------------------------------------------*/
#ifndef application_h
#define application_h

#include "Server.h"
#include "Client.h"
#include "EZ_UI.h"

/*---------------------------------------------------------------------------------------
--	Global declarations for the window handles to the UI elements. These accessed
--	at many points in the application and I have decided to make them global declarations
--	for that reason.
---------------------------------------------------------------------------------------*/
HWND hwnd;
HWND editCtlIP;
HWND editCtlPort;
HWND btn;
HWND labelIP;
HWND labelPort;
HWND result;
HWND radioTCP;
HWND radioUDP;
HWND dropDown1;
HWND labelSize;
HWND labelNumTimes;
HWND dropDown2;
HWND dropDownDelay;
HWND labelDelay;
//Device Context
HDC hdc;

int mode;

//Function Headers
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void InstantiateWindow(HINSTANCE);
void CheckMenu(WPARAM wP);
void InstantiateUI();
void UpdateUI(int);
void Resolve(int);
void ClearText();
void CommandUIstate();
void ClientUIstate();
void ServerUIstate();
void UIControl();
void PopulateUIElements();

#endif
