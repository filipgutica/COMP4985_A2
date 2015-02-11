/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	Application.h - Header file for Application.cpp
--									Contains function headers, and Necesarry 
--									global declarations such as the handles to the
--									UI elements.
--
--	PROGRAM:		Network_Resolver
--
--
--	DATE:			February 9, 2015
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
