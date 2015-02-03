/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		tcp_clnt.c - A simple TCP client program.
--
--	PROGRAM:			tclnt.exe
--
--	FUNCTIONS:			Winsock 2 API
--
--	DATE:				January 11, 2006
--
--	REVISIONS:			(Date and Description)
--
--						Oct. 1, 2007 (A. Abdulla):
--						
--						Changed the read loop to better handle the 
--						blocking recv call. 
--
--	DESIGNERS:			Aman Abdulla
--
--	PROGRAMMERS:		Aman Abdulla
--
--	NOTES:
--	The program will establish a TCP connection to a user specifed server.
--  The server can be specified using a fully qualified domain name or and
--	IP address. After the connection has been established the user will be
--  prompted for date. The date string is then sent to the server and the
--  response (echo) back from the server is displayed.
---------------------------------------------------------------------------------------*/
#include "Client.h"

IO_DATA ioInfo;

char *host;
int port;

void StartClient (char *ip, char *p, int size, int numTimes, char *protocol, HWND mainHwnd, HWND resultHwnd)
{
	

	ioInfo.hWndResult = resultHwnd;
	ioInfo.hWnd = mainHwnd;
	ioInfo.size = size;
	ioInfo.numtimes = numTimes;
	ioInfo.protocol = protocol;
		
	host =	ip;
	port =	atoi(p);	// User specified port
	

	if (strcmp(ioInfo.protocol, "tcp") == 0)
		StartTCP();
	//else if (strcmp(ioInfo.protocol, "udp") == 0)
		//StartUDP();

	
	
}

void StartTCP()
{
	int err;
	struct hostent	*hp;
	struct sockaddr_in server;
	char **pptr;
	WSADATA WSAData;
	WORD wVersionRequested;
	DWORD ThreadId;
	int i = 0;
	char temp[1024];

	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &WSAData );
	if ( err != 0 ) //No usable DLL
	{
		printf ("DLL not found!\n");
		exit(1);
	}

	// Create the socket
	if ((ioInfo.sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
		exit(1);
	}

	// Initialize and set up the address structure
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}

	// Copy the server address
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// Connecting to the server
	if (connect (ioInfo.sock, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		sprintf(temp, "Can't connect to server\n");
		SetWindowText(ioInfo.hWndResult, temp);
		perror("connect");
		
	}



	sprintf(temp, "Connected:    Server Name: %s\n \t\tIP Address: %s\n", hp->h_name, inet_ntoa(server.sin_addr));
	SetWindowText(ioInfo.hWndResult, temp);
	pptr = hp->h_addr_list;
	printf("Transmiting:\n");
	

	if (CreateThread(NULL, 0, ProcessClientIO, (LPVOID)ioInfo.hWnd, 0, &ThreadId) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return ;
	} 
}

DWORD WINAPI ProcessClientIO(LPVOID lpParameter)
{
	char *rbuf = new char[64];
	char sbuf[BUFSIZE] = "\0";
	char *bp;
	char temp[128];
	HWND h = (HWND)lpParameter;
	int ns = 0; 
	int n, bytes_to_read;
	int i = 0;
	struct hostent	*hp;
	struct sockaddr_in server;
	SOCKET dataSock;

	int totalSize = ioInfo.size * ioInfo.numtimes;
	sprintf(sbuf, "%d", totalSize);

	ns = send (ioInfo.sock, sbuf, strlen(sbuf), 0);

	memset((char*)sbuf, 0, sizeof(sbuf));

	while (TRUE)
	{
		memset((char *)sbuf, 0, sizeof(sbuf));

		// Transmit data through the socket
		
		bp = rbuf;
		bytes_to_read = BUFSIZE;

		// client makes repeated calls to recv until no more data is expected to arrive.
	
		n = recv (ioInfo.sock, bp, 1024, 0);
	
		if (atoi(rbuf) == ACK)
		{
			char receiveBuf[64];
			clock_t t;

			t = clock();
			//MessageBox(NULL, "GOT ACK", "", MB_OK);
			n = 0;
			if ((dataSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			{
				perror("Cannot create socket");
				//exit(1);
			}

			// Initialize and set up the address structure
			memset((char *)&server, 0, sizeof(struct sockaddr_in));
			server.sin_family = AF_INET;
			server.sin_port = htons(port);
			if ((hp = gethostbyname(host)) == NULL)
			{
				fprintf(stderr, "Unknown server address\n");
				//exit(1);
			}

			// Copy the server address
			memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

			// Connecting to the server
			if (connect (dataSock, (struct sockaddr *)&server, sizeof(server)) == -1)
			{
				fprintf(stderr, "Can't connect to server\n");
				perror("connect");
				SendMessage(h, WM_DISCONNECT, 0, 0);
			}

			
			
			for (int i = 0; i < ioInfo.numtimes; i++)
			{
				if (i == (ioInfo.numtimes - 1))
				{
					sprintf(sbuf, "FIN");
					ns = send(dataSock, sbuf, ioInfo.size, 0);
				}
				else
				{
					sprintf(sbuf, "");
					ns = send (dataSock, sbuf, ioInfo.size, 0);
				}
					n = recv(dataSock, bp, 1024, 0);
			}

			t = clock() - t;
			
			sprintf(temp, "Sent: %d bytes \t Server got %d bytes \t time: %f sec", totalSize, atoi(rbuf), ((float)t/CLOCKS_PER_SEC));
			SetWindowText(ioInfo.hWndResult, temp);

			break;
		}
			
			
			

		
	}

	Sleep(1000);
	closesocket (ioInfo.sock);
	closesocket(dataSock);
	WSACleanup();
	SendMessage(h, WM_DISCONNECT, 0, 0);
	return 0;

}
