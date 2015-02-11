/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		Client.cpp - Client implementation, uses TCP or UDP depending 
--									 user input. Implmeneted using the Overlapped I/O model
--
--	PROGRAM:			Assignment2.exe
--
--	FUNCTIONS:			Winsock 2 API
--
--	DATE:				Febuary 9 2015
--
--	REVISIONS:			(Date and Description)
--
--	DESIGNERS:			Filip Gutica
--
--	PROGRAMMERS:		Filip Gutica
--
--	NOTES:
--	The program will establish a TCP connection to a user specifed server.
--  The program will then send a user-specified packet size, a user-specified
--	number of times. The client will either use UDP or TCP as the client can 
--	receive both. A TCP connection is established regardless of protocol as at minimum
--	one TCP socket will be used for transferring control characters.
---------------------------------------------------------------------------------------*/
#include "Client.h"

IO_DATA ioInfo;
TCP_STATS tcpStats;
UDP_STATS udpStats;
struct	hostent	*hp;
struct	sockaddr_in server, client;

/*------------------------------------------------------------------------------
--	FUNCTION: StartClient()
--
--	PURPOSE:		Initializes the client and sets all necesarry parameters.
--
--	PARAMETERS:
--		CHAR *ip			-IP address
--		CHAR *p				-Port number
--		INT size			-Packet size
--		INT numTimes		-Number of times to send
--		CHAR *protocol		-Protocol to be used
--		CHAR *delay			-Delay for writing to server
--		HWND mainHwnd		-Handle to the main window for sending any messages
--		HWND resultHwnd		-Handle to the result box where I will display logs and info
--		HANDLE h1			-File handle for a file to write results to
--		HANDLE h2			-File handle for a file to write results to
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
void StartClient (char *ip, char *p, int size, int numTimes, char *protocol, char *delay, HWND mainHwnd, HWND resultHwnd, HANDLE h1, HANDLE h2)
{
	ioInfo.hWndResult = resultHwnd;
	ioInfo.hWnd = mainHwnd;
	ioInfo.size = size;
	ioInfo.numtimes = numTimes;
	ioInfo.protocol = protocol;
	ioInfo.ip =	ip;
	ioInfo.port = atoi(p);	// User specified port
	ioInfo.delay = atoi(delay);
	ioInfo.fLostBytes = h1;
	ioInfo.fTime = h2;
	//Start TCP as we will always have a TCP control channel
	StartTCP();
}

/*------------------------------------------------------------------------------
--	FUNCTION: StartTCP()
--
--	PURPOSE:		Creates a TCP socket and connects to the server.
--
--	PARAMETERS:
--		void
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
void StartTCP()
{
	INT err;
	struct hostent	*hInfo;
	struct sockaddr_in serv;
	CHAR **pptr;
	WSADATA WSAData;
	WORD wVersionRequested;
	DWORD ThreadId;
	INT i = 0;
	CHAR temp[TEMP_BUFSIZE];

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

/*------------------------------------------------------------------------------
--	FUNCTION: ProcessClientIO()
--
--	PURPOSE:		Thread to facilitate client UDP or TCP I/O 
--
--	PARAMETERS:
--		LPVOID lpParameter			-Parameter to be passed to the thread
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
DWORD WINAPI ProcessClientIO(LPVOID lpParameter)
{
	char temp[TEMP_BUFSIZE];
	DWORD dwWritten;
	if (strcmp(ioInfo.protocol, "udp") == 0)
	{
		UDP();

		sprintf(temp, " %d, %f\n", udpStats.totalSizeSent , 
			((float)udpStats.lostBytes/(float)udpStats.totalSizeSent) * 100.0);

		SetFilePointer( ioInfo.fLostBytes, 0, NULL, FILE_END);
		WriteFile(ioInfo.fLostBytes, temp,strlen(temp),&dwWritten,NULL);


		sprintf(temp, " %d, %f\n", udpStats.totalSizeSent , udpStats.timeRaw);

		SetFilePointer( ioInfo.fLostBytes, 0, NULL, FILE_END);
		WriteFile(ioInfo.fTime, temp,strlen(temp),&dwWritten,NULL);
	}
	else if (strcmp(ioInfo.protocol, "tcp") == 0)
	{
		TCP();

		sprintf(temp, " %d, %f\n", tcpStats.totalSizeSent , 
			((float)tcpStats.lostBytes/(float)tcpStats.totalSizeSent) * 100.0);

		SetFilePointer( ioInfo.fLostBytes, 0, NULL, FILE_END);
		WriteFile(ioInfo.fLostBytes, temp,strlen(temp),&dwWritten,NULL);


		sprintf(temp, " %d, %f\n", tcpStats.totalSizeSent , tcpStats.timeRaw);

		SetFilePointer( ioInfo.fLostBytes, 0, NULL, FILE_END);
		WriteFile(ioInfo.fTime, temp,strlen(temp),&dwWritten,NULL);
	}

	WSACleanup();
	return 0;
}

/*------------------------------------------------------------------------------
--	FUNCTION: UDP()
--
--	PURPOSE:		Function to facilitate UDP I/O. Here I send packets to the server 
--
--	PARAMETERS:
--		void
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
void UDP()
{
	DWORD Index;
	DWORD BytesTransferred = 0;
	DWORD Flags;
	SOCKET DataSocket;
	WSAOVERLAPPED ol;
	WSABUF DataBuff;
	WSABUF ControlBuff;
	char *Buffer;
	char temp[TEMP_BUFSIZE];
	udpStats.totalSizeSent = ioInfo.size * ioInfo.numtimes;
	DWORD TotalBytesSent;
	DWORD BytesRecv;
	clock_t t;
	
	if ((ol.hEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent failed with error %d\n", WSAGetLastError());
		return;
	}
	
	/* Tell the server what protocol were using */
	sprintf(temp, "%s", ioInfo.protocol);
	ControlBuff.buf = temp;
	ControlBuff.len = strlen(temp);
	ClientWriteSocket(&ioInfo.sock, &ControlBuff, &ol);

	DataSocket = CreateUDPSocket();

	Buffer = (char*)malloc(ioInfo.size * sizeof(char));

	t = clock();
	
	/* Send the data */
	for (int i = 0; i < ioInfo.numtimes; i++)
	{
		if ((Index = WSAWaitForMultipleEvents(1, &(ol.hEvent), FALSE,
			WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			sprintf(temp, "WSAWaitForMultipleEvents failed %d\n", WSAGetLastError());
			return;
		} 

		WSAResetEvent(ol.hEvent);

		if (WSAGetOverlappedResult(DataSocket, &ol, &BytesTransferred, FALSE, &Flags) == FALSE)
		{
			break;
		}

		TotalBytesSent += BytesTransferred;

		int k = 0;
		for (int j = 0; j < ioInfo.size; j++)
		{
			k = (j < 26) ? j : j % 26;
			Buffer[j] = 'a' + k;
		}
		//sprintf(Buffer, "hello");
		DataBuff.buf = Buffer;
		DataBuff.len = ioInfo.size;
		Sleep(ioInfo.delay);
		ClientWriteUDPSocket(&DataSocket, &DataBuff, &ol);
		
	}
	
	/* Send and listen for control characters */
	while (TRUE)
	{
		if ((Index = WSAWaitForMultipleEvents(1, &(ol.hEvent), FALSE,
		WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents failed %d\n", WSAGetLastError());
			return;
		} 

		if (WSAGetOverlappedResult(ioInfo.sock, &ol, &BytesTransferred, FALSE, &Flags) == FALSE)
		{
			break;
		}

		/*If BytesRecv is 0 then the WSARecv call finished and the server has responed that
		  it received x amount of bytes.*/
		if (BytesRecv == 0)
			break;

		Sleep(ioInfo.delay);
		sprintf(temp, "%d", EOT);
		ControlBuff.buf = temp;
		ControlBuff.len = strlen(temp);
		ClientWriteSocket(&ioInfo.sock, &ControlBuff, &ol);

		Flags = 0;
		ControlBuff.buf = temp;
		ControlBuff.len = sizeof(temp);
		BytesRecv = 0;
		ClientReadSocket(&ioInfo.sock, &ControlBuff, Flags, &ol);
	}

	t = clock() - t;

	/* Populate UDP stats structure for reporting */
	udpStats.timeRaw = ((float)t/CLOCKS_PER_SEC) - ((ioInfo.delay * ioInfo.numtimes)/CLOCKS_PER_SEC);
	udpStats.timeDelayed = ((float)t/CLOCKS_PER_SEC);
	udpStats.bytesReceived = atoi(temp);
	udpStats.lostBytes = udpStats.totalSizeSent - udpStats.bytesReceived;

	/* Display results to the screen */
	sprintf(temp, "Sent: %d bytes \t Server got %d bytes \t time: %f sec \ttime(minus delay): %f \t Protocol: %s\n",
		udpStats.totalSizeSent, udpStats.bytesReceived, udpStats.timeDelayed, udpStats.timeRaw, ioInfo.protocol);
	SetWindowText(ioInfo.hWndResult, temp);

	Sleep(1000);
	closesocket(DataSocket);
	closesocket(ioInfo.sock);
}

/*------------------------------------------------------------------------------
--	FUNCTION: TCP()
--
--	PURPOSE:		Function to facilitate TCP I/O. Here I send packets to the server 
--
--	PARAMETERS:
--		void
--
--	DESIGNERS:		Filip Gutica
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
void TCP()
{
	DWORD Index;
	DWORD BytesTransferred;
	DWORD Flags;
	SOCKET DataSocket;
	WSAOVERLAPPED ol;
	WSABUF DataBuff;
	WSABUF ControlBuff;
	char *Buffer;
	char temp[BUFSIZ];
	tcpStats.totalSizeSent = ioInfo.size * ioInfo.numtimes;
	DWORD TotalBytesSent;
	DWORD BytesRecv;
	clock_t t;
	
	if ((ol.hEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent failed with error %d\n", WSAGetLastError());
		return;
	}

	/* Tell the server what protocol were using */
	sprintf(temp, "%s", ioInfo.protocol);
	ControlBuff.buf = temp;
	ControlBuff.len = strlen(temp);
	ClientWriteSocket(&ioInfo.sock, &ControlBuff, &ol);

	DataSocket = CreateTCPSocket();

	Buffer = (char*)malloc(ioInfo.size * sizeof(char));

	t = clock();

	/* Send the data */
	for (int i = 0; i < ioInfo.numtimes; i++)
	{
		if ((Index = WSAWaitForMultipleEvents(1, &(ol.hEvent), FALSE,
			WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents failed %d\n", WSAGetLastError());
			return;
		} 

		if (WSAGetOverlappedResult(DataSocket, &ol, &BytesTransferred, FALSE, &Flags) == FALSE)
		{
			break;
		}

		int k = 0;
		for (int j = 0; j < ioInfo.size; j++)
		{
			k = (j < 26) ? j : j % 26;
			Buffer[j] = 'a' + k;
		}
		DataBuff.buf = Buffer;
		DataBuff.len = ioInfo.size;
		ClientWriteSocket(&DataSocket, &DataBuff, &ol);
		Sleep(ioInfo.delay);
	}

	/* Send and listen for control characters */
	while (TRUE)
	{
		if ((Index = WSAWaitForMultipleEvents(1, &(ol.hEvent), FALSE,
			WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents failed %d\n", WSAGetLastError());
			return;
		} 

		if (WSAGetOverlappedResult(ioInfo.sock, &ol, &BytesTransferred, FALSE, &Flags) == FALSE)
		{
			break;
		}
	
		/*If BytesRecv is 0 then the WSARecv call finished and the server has responed that
		  it received x amount of bytes.*/
		if (BytesRecv == 0)
			break;

		sprintf(temp, "%d", EOT);
		ControlBuff.buf = temp;
		ControlBuff.len = strlen(temp);
		ClientWriteSocket(&ioInfo.sock, &ControlBuff, &ol);


		Flags = 0;
		ControlBuff.buf = temp;
		ControlBuff.len = sizeof(temp);
		BytesRecv = 0;
		ClientReadSocket(&ioInfo.sock, &ControlBuff, Flags, &ol);
	}

	t = clock() - t;

	/* Populate TCP stats structure for reporting */
	tcpStats.timeRaw = ((float)t/CLOCKS_PER_SEC) - ((ioInfo.delay * ioInfo.numtimes)/CLOCKS_PER_SEC);
	tcpStats.timeDelayed = ((float)t/CLOCKS_PER_SEC);
	tcpStats.bytesReceived = atoi(temp);
	tcpStats.lostBytes = tcpStats.totalSizeSent - tcpStats.bytesReceived;

	/* Display results to the screen */
	sprintf(temp, "Sent: %d bytes \t Server got %d bytes \t time: %f sec \ttime(minus delay): %f \t Protocol: %s\n",
		tcpStats.totalSizeSent, tcpStats.bytesReceived, tcpStats.timeDelayed, tcpStats.timeRaw, ioInfo.protocol);
	SetWindowText(ioInfo.hWndResult, temp);

	Sleep(1000);
	closesocket(ioInfo.sock);
	closesocket(DataSocket);
}

/*------------------------------------------------------------------------------
--	FUNCTION: CreateTCPSocket()
--
--	PURPOSE:		Wrapper function for creating WSASockets for TCP.  
--
--	PARAMETERS:
--			void
--
--	Return:
			SOCKET	-a new SOCKET is returned if succesfull.
--
--	DESIGNERS:		Filip Gutica 
--
--	PROGRAMMER:		Filip Gutica 
--
--	Notes:	Used code from class by Aman Abdulla to help code this function.
/*-----------------------------------------------------------------------------*/
SOCKET CreateTCPSocket()
{
	SOCKET temp;

	if ((temp = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
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
		return 0;
	}

	// Copy the server address
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// Connecting to the server
	if (connect (temp, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		return 0;
	}
	
	return temp;
			
}

/*------------------------------------------------------------------------------
--	FUNCTION: CreateUDPSocket()
--
--	PURPOSE:		Wrapper function for creating WSASockets for UDP.  
--
--	PARAMETERS:
--			void
--
--	Return:
			SOCKET	-a new SOCKET is returned if succesfull.
--
--	DESIGNERS:		Filip Gutica 
--
--	PROGRAMMER:		Filip Gutica 
--
--	Notes:	Used code from class by Aman Abdulla to help code this function.
/*-----------------------------------------------------------------------------*/
SOCKET CreateUDPSocket()
{
	SOCKET temp;

	// Create a datagram socket
	if ((temp = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		perror ("Can't create a socket\n");
		exit(1);
	}

	// Initialize and set up the address structure
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(ioInfo.port);

	if ((hp = gethostbyname(ioInfo.ip)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		return 0;
	}

	// Copy the server address
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	return temp;
}

/*------------------------------------------------------------------------------
--	FUNCTION: ClientWriteSocket()
--
--	PURPOSE:		Wrapper function for writing data to a WSASocket using 
--					WSASend and overlapped structures
--
--	PARAMETERS:
--			SOCKET *sock		-Socket to write to
--			WSABUF *buf			-Buffer to be written
--			WSAOVERLAPPED *ol	-Overlapped structure to be used for Overlapped I/O
--
--	Return:
			DWORD	Number of bytes written.
--
--	DESIGNERS:		Filip Gutica 
--
--	PROGRAMMER:		Filip Gutica 
--
/*-----------------------------------------------------------------------------*/
DWORD ClientWriteSocket(SOCKET *sock, WSABUF *buf, WSAOVERLAPPED *ol)
{
	DWORD sb;
	char temp[64];

	if (WSASend(*sock, buf, 1, &sb, 0,
		ol, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			sprintf(temp, "WSASend() failed with error %d", WSAGetLastError());
			MessageBox(NULL, temp, "", MB_OK);
			return 0;
		}
	}

	return sb;
}

/*------------------------------------------------------------------------------
--	FUNCTION: ClientWriteUDPSocket()
--
--	PURPOSE:		Wrapper function for writing data to a WSASocket using 
--					WSASendTo and overlapped structures
--
--	PARAMETERS:
--			SOCKET *sock		-Socket to write to
--			WSABUF *buf			-Buffer to be written
--			WSAOVERLAPPED *ol	-Overlapped structure to be used for Overlapped I/O
--
--	Return:
			DWORD	Number of bytes written.
--
--	DESIGNERS:		Filip Gutica 
--
--	PROGRAMMER:		Filip Gutica 
--
/*-----------------------------------------------------------------------------*/
DWORD ClientWriteUDPSocket(SOCKET *sock, WSABUF *buf, WSAOVERLAPPED *ol)
{
	DWORD sb;
	char temp[64];

	if (WSASendTo(*sock, buf, 1, &sb, 0,
		(PSOCKADDR)&server,sizeof(server), ol, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			sprintf(temp, "WSASend() failed with error %d", WSAGetLastError());
			MessageBox(NULL, temp, "", MB_OK);
			return 0;
		}
	}

	return sb;
}

/*------------------------------------------------------------------------------
--	FUNCTION: ClientReadSocket()
--
--	PURPOSE:		Wrapper function for receiving data from a WSASocket using 
--					WSARecv and overlapped structures
--
--	PARAMETERS:
--			SOCKET *sock		-Socket to receive from
--			WSABUF *buf			-Receive buffer
--			DWORD fl			-Flags
--			WSAOVERLAPPED *ol	-Overlapped structure to be used for Overlapped I/O
--
--	Return:
			DWORD	Number of bytes written.
--
--	DESIGNERS:		Filip Gutica 
--
--	PROGRAMMER:		Filip Gutica 
--
/*-----------------------------------------------------------------------------*/
DWORD ClientReadSocket(SOCKET *sock, WSABUF *buf, DWORD fl,  WSAOVERLAPPED *ol)
{
	DWORD rb;
	char temp[64];

	if (WSARecv(*sock, buf, 1, &rb, &fl,
			ol, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			sprintf(temp, "WSASRecv() failed with error %d", WSAGetLastError());
			MessageBox(NULL, temp, "", MB_OK);
			return 0;
		}
			
	}

	return rb;
}