/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	Client.h -		Header file for Client.cpp
--									Contains function headers and Structures
--									used by Client.cpp
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
#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
#include <time.h>
#include <string>
#include <string.h>

#define PORT				7575	// Default port
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
	HANDLE fLostBytes;
	HANDLE fTime;
};

typedef struct UDP_STATS
{
	int totalSizeSent;
	int bytesReceived;
	float timeDelayed;
	float timeRaw;
	unsigned lostBytes;
};

typedef struct TCP_STATS
{
	int totalSizeSent;
	int bytesReceived;
	float timeDelayed;
	float timeRaw;
	unsigned lostBytes;
};

//Functions headers
void StartClient (char *ip, char *p,int, int, char*, char*, HWND mainHwnd, HWND resultHwnd, HANDLE, HANDLE);
void StartTCP();
void TCP();
void UDP();
SOCKET CreateTCPSocket();
SOCKET CreateUDPSocket();
DWORD WINAPI ProcessClientIO(LPVOID lpParameter);
DWORD ClientReadSocket(SOCKET *sock, WSABUF *buf, DWORD fl,  WSAOVERLAPPED *ol);
DWORD ClientWriteSocket(SOCKET *sock, WSABUF *buf, WSAOVERLAPPED *ol);
DWORD ClientWriteUDPSocket(SOCKET *sock, WSABUF *buf, WSAOVERLAPPED *ol);

#endif