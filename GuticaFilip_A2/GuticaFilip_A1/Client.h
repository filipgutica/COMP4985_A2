#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
#include <time.h>
//#include <string.h>
//#include <memory.h>

#define SERVER_TCP_PORT			7000	// Default port
#define BUFSIZE					65535		// Buffer length
#define ACK						06
#define EOT						04

#define WM_CONNECTED (WM_USER + 0x0001)
#define WM_DISCONNECT (WM_USER + 0x002)

typedef struct IO_DATA 
{
	HWND hWnd;
	HWND hWndResult;
	SOCKET sock;
	int size;
	int numtimes;
	char* protocol;
};

void StartClient (char *ip, char *p,int, int, char*, HWND mainHwnd, HWND resultHwnd);
void StartTCP();
DWORD WINAPI ProcessClientIO(LPVOID lpParameter);

#endif