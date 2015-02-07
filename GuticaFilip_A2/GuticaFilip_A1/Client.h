#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
#include <time.h>
#include <string>
#include <string.h>
//#include <string.h>
//#include <memory.h>

#define PORT				7000	// Default port
#define UDP_PORT			7001
#define BUFSIZE				65535		// Buffer length
#define TEMP_BUFSIZE		256
#define ACK					06
#define EOT					04

#define WM_CONNECTED (WM_USER + 0x0001)
#define WM_DISCONNECT (WM_USER + 0x002)

typedef struct IO_DATA 
{
	HWND hWnd;
	HWND hWndResult;
	SOCKET sock;
	int size;
	int numtimes;
	int port;
	int delay;
	char* ip;
	char* protocol;
};

void StartClient (char *ip, char *p,int, int, char*, char*, HWND mainHwnd, HWND resultHwnd);
void StartTCP();
void TCP();
void UDP();
SOCKET CreateTCPSocket();
SOCKET CreateUDPSocket();
DWORD WINAPI ProcessClientIO(LPVOID lpParameter);

#endif