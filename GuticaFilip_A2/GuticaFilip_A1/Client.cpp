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
struct	hostent	*hp;
struct	sockaddr_in server, client;

void StartClient (char *ip, char *p, int size, int numTimes, char *protocol, HWND mainHwnd, HWND resultHwnd)
{
	ioInfo.hWndResult = resultHwnd;
	ioInfo.hWnd = mainHwnd;
	ioInfo.size = size;
	ioInfo.numtimes = numTimes;
	ioInfo.protocol = protocol;
		
	ioInfo.ip =	ip;
	ioInfo.port = atoi(p);	// User specified port
	
	StartTCP();

}

void StartTCP()
{
	int err;
	struct hostent	*hInfo;
	struct sockaddr_in serv;
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

	ioInfo.sock = CreateTCPSocket();

	sprintf(temp, "Connected:    Server Name: %s\n \t\tIP Address: %s\n", hp->h_name, inet_ntoa(server.sin_addr));
	SetWindowText(ioInfo.hWndResult, temp);
	pptr = hp->h_addr_list;
	
	if (CreateThread(NULL, 0, ProcessClientIO, (LPVOID)ioInfo.hWnd, 0, &ThreadId) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return ;
	} 
}

DWORD WINAPI ProcessClientIO(LPVOID lpParameter)
{
	if (strcmp(ioInfo.protocol, "udp") == 0)
	{
		UDP();
	}
	else if (strcmp(ioInfo.protocol, "tcp") == 0)
	{
		TCP();
	}

	WSACleanup();
	return 0;
}


void UDP()
{
	int	data_size = ioInfo.size;
	int port = ioInfo.port;
	int	i, j, server_len, client_len;
	SOCKET DataSocket;
	char *pname, *host, rbuf[BUFSIZ], sbuf[BUFSIZ];

	SYSTEMTIME stStartTime, stEndTime;
	WSADATA stWSAData;

	sprintf(sbuf, "%s", ioInfo.ip);

	send (ioInfo.sock, sbuf, strlen(sbuf), 0);


	DataSocket = CreateUDPSocket();

}

void TCP()
{
	char rbuf[64];
	char sbuf[BUFSIZE] = "\0";
	char *bp;
	char temp[128];
	int ns = 0; 
	int n, bytes_to_read;
	int i = 0;
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

			dataSock = CreateTCPSocket();
		
			for (int i = 0; i < ioInfo.numtimes; i++)
			{
				if (i == (ioInfo.numtimes - 1))
				{
					sprintf(sbuf, "%d", EOT);
					n = send(dataSock, sbuf, ioInfo.size, 0);
				}
				else
				{
					sprintf(sbuf, "");
					n = send (dataSock, sbuf, ioInfo.size, 0);
				}
				n = recv(dataSock, bp, 1024, 0);
				SetWindowText(ioInfo.hWndResult, TEXT("Transmitting..."));
			}

			t = clock() - t;
			
			sprintf(temp, "Sent: %d bytes \t Server got %d bytes \t time: %f sec", totalSize, atoi(rbuf), ((float)t/CLOCKS_PER_SEC));
			SetWindowText(ioInfo.hWndResult, temp);

			break;
		}
	}

	//Give the server a chance to finish sending then close sockets
	Sleep(1000);
	closesocket (ioInfo.sock);
	closesocket(dataSock);
}

SOCKET CreateTCPSocket()
{
	SOCKET temp;

	if ((temp = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
		//exit(1);
	}

	// Initialize and set up the address structure
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(ioInfo.port);
	if ((hp = gethostbyname(ioInfo.ip)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		//exit(1);
	}

	// Copy the server address
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// Connecting to the server
	if (connect (temp, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
	}
	
	return temp;
			
}

SOCKET CreateUDPSocket()
{
	SOCKET temp;


	// Create a datagram socket
	if ((temp = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror ("Can't create a socket\n");
		exit(1);
	}

	// Store server's information
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(ioInfo.port);

	if ((hp = gethostbyname(ioInfo.ip)) == NULL)
	{
		fprintf(stderr,"Can't get server's IP address\n");
		exit(1);
	}
	//strcpy((char *)&server.sin_addr, hp->h_addr);
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// Bind local address to the socket
	memset((char *)&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(0);  // bind to any available port
	client.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(temp, (struct sockaddr *)&client, sizeof(client)) == -1)
	{
		perror ("Can't bind name to socket");
		exit(1);
	}

	return temp;
}