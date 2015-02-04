#ifndef SERV_H
#define SERV_H

#include <winsock2.h>
#include <stdio.h>
#include <string>
#include <vector>

#define PORT			7000
#define DATA_BUFSIZE	65535	//Maximum size to receive on TCP one packet	
#define BUFFER_SIZE		65535	// "" 
#define TEMP_BUFSIZE	64
#define ACK				06
#define EOT				04

using namespace std;

typedef struct _SOCKET_INFORMATION {
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   SOCKET Socket;
   WSAOVERLAPPED Overlapped;
   DWORD BytesSEND;
   DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

//Functions
DWORD WINAPI ProcessIO(LPVOID lpParameter);
DWORD WINAPI ListenThread(LPVOID lpParameter);
void StartServer(HWND);
void PrintIOLog(vector<string> v , HWND h);
DWORD WriteToSocket(SOCKET sock, WSABUF buf, WSAOVERLAPPED overlapped);
DWORD ReadSocket(SOCKET *sock, WSABUF *buf, DWORD fl,  WSAOVERLAPPED *ol);
void StopServer();

#endif