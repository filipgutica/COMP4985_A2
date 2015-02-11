/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		Server.cpp - TCP and UDP server using the Overlapped I/O model.
--									 This application listens for TCP connections then
--									 listens for what protocol will be used, begins listening
--									 for data, upon receiving an EOT it will send back to the
--									 client how many bytes were received.
--
--	PROGRAM:			Assignment2.exe
--
--	FUNCTIONS:			StartServer
--						ListenThread
--						ProcessTCP_IO
--						ProcessUDP_IO
--						PrintIOLog
--						WriteTOSocket
--						ReadSocket
--
--	DATE:				Febuary 9 2015
--
--	DESIGNERS:			Filip Gutica
--
--	PROGRAMMERS:		Filip Gutica
--
--	NOTES:
--	This program will create a UDP socket and TCP listen socket and listen for any connections.
--	When it receives a connection, it will listen for what protocol is being used.
--  If it gets TCP it will continue as it was. If it gets UDP it will also start the UDP IO
--  thread to process UDP IO. Upon receiving an EOT on the TCP channel, the server will send
--  back to the client how many bytes it received.
--
--	I used the sample code given to us in class called overlap.cpp in order to help me with
--	implementing this server.
--
---------------------------------------------------------------------------------------*/
#include "Server.h"

DWORD EventTotal = 0;
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION CriticalSection;   
SOCKET ListenSocket, AcceptSocket, ControlSocket, UDPSock;
HANDLE hThrdIO, hThrdListen, hThrdUDP;
vector<string> infoVector;
int Mode;
int TotalTCPBytes = 0;
int TotalUDPBytes = 0;


/*------------------------------------------------------------------------------
--	FUNCTION: StartServer()
--
--	PURPOSE:		Initializes the server. Creates the UDP socket and TCP listen
--					sockets and starts the listen and process IO threads.
--
--	PARAMETERS:
--		HWND h		-handle to the results textbox to display log info.
--
--	DESIGNERS:		Filip Gutica & In class example overlap.cpp
--
--	PROGRAMMER:		Filip Gutica & In class example overlap.cpp
/*-----------------------------------------------------------------------------*/
void StartServer(HWND h)
{
	WSADATA wsaData;
	SOCKADDR_IN InternetAddr;
	DWORD Flags;
	DWORD IOThreadId;
	DWORD ListenThreadId;
	DWORD RecvBytes;
	INT Ret;
	char temp[256];

	InitializeCriticalSection(&CriticalSection);

	if ((Ret = WSAStartup(0x0202,&wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		WSACleanup();
		return;
	}

	if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return;
	}

	if ((UDPSock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, 
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{
		sprintf(temp, "Failed to get a socket %d\n", WSAGetLastError());
		infoVector.push_back(temp);
		PrintIOLog(infoVector, h);
		return;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);

	if (bind(ListenSocket, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() listensocket failed with error %d\n", WSAGetLastError());
		PrintIOLog(infoVector, h);
		return;
	}

	if (bind(UDPSock, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		sprintf(temp, "bind() udpsocket failed with error %d\n", WSAGetLastError());
		PrintIOLog(infoVector, h);
		infoVector.push_back(temp);
		return;
	}

	if (listen(ListenSocket, 5))
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		return;
	}

	// Setup the listening socket for connections.

	if ((AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return;
	}

	if ((EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent failed with error %d\n", WSAGetLastError());
		return;
	}

	// Create a thread to service overlapped requests and I/O operations

	if ((hThrdIO = CreateThread(NULL, 0, ProcessTCP_IO, (LPVOID)h, 0, &IOThreadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return;
	} 

	//Create a thread to Listen for incomming connections

	if ((hThrdListen = CreateThread(NULL, 0, ListenThread, (LPVOID)h, 0, &ListenThreadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return;
	} 

	EventTotal = 1;
}


/*------------------------------------------------------------------------------
--	FUNCTION: ListenThread()
--
--	PURPOSE:		Listens for incomming connections
--
--	PARAMETERS:
--		LPVOID lpParameter		-Parameter for the thread
--
--	DESIGNERS:		Filip Gutica & In class example overlap.cpp
--
--	PROGRAMMER:		Filip Gutica & In class example overlap.cpp
/*-----------------------------------------------------------------------------*/
DWORD WINAPI ListenThread(LPVOID lpParameter)
{
	DWORD Flags;
	DWORD RecvBytes;
	HWND hwnd = (HWND)lpParameter;
	char temp[TEMP_BUFSIZE];

	 while(TRUE)
   {
       // Accept inbound connections

      if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) == INVALID_SOCKET)
      {
          printf("accept failed with error %d\n", WSAGetLastError());
          return 0;
      }

      EnterCriticalSection(&CriticalSection);

      // Create a socket information structure to associate with the accepted socket.

      if ((SocketArray[EventTotal] = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR,
         sizeof(SOCKET_INFORMATION))) == NULL)
      {
         printf("GlobalAlloc() failed with error %d\n", GetLastError());
         return 0;
      } 

	   sprintf(temp, "Accepted connection: %d", AcceptSocket);
	   infoVector.push_back(temp);

      // Fill in the details of our accepted socket
	  SocketArray[EventTotal]->Socket = AcceptSocket;
      ZeroMemory(&(SocketArray[EventTotal]->Overlapped), sizeof(OVERLAPPED));
      SocketArray[EventTotal]->BytesSEND = 0;
      SocketArray[EventTotal]->BytesRECV = 0;
      SocketArray[EventTotal]->DataBuf.len = DATA_BUFSIZE;
      SocketArray[EventTotal]->DataBuf.buf = SocketArray[EventTotal]->Buffer;

      if ((SocketArray[EventTotal]->Overlapped.hEvent = EventArray[EventTotal] = 
          WSACreateEvent()) == WSA_INVALID_EVENT)
      {
         printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
         return 0;
      }

      //Read the socket to begin receiving data
      Flags = 0;
	  RecvBytes = ReadSocket(&SocketArray[EventTotal]->Socket, 
		  &(SocketArray[EventTotal]->DataBuf), Flags, 
		  &(SocketArray[EventTotal]->Overlapped));

      EventTotal++;

      LeaveCriticalSection(&CriticalSection);

      //
      // Signal the first event in the event array to tell the worker thread to
      // service an additional event in the event array
      //
      if (WSASetEvent(EventArray[0]) == FALSE)
      {
         printf("WSASetEvent failed with error %d\n", WSAGetLastError());
         return 0;
      }
   }

}

/*------------------------------------------------------------------------------
--	FUNCTION: ProcessTCP_IO()
--
--	PURPOSE:		Processes IO on the TCP channel
--
--	PARAMETERS:
--		LPVOID lpParameter		-Parameter for the thread, Will be the
--								 handle to the window where results are to be displayed
--
--	DESIGNERS:		Filip Gutica & In class example overlap.cpp
--
--	PROGRAMMER:		Filip Gutica & In class example overlap.cpp
/*-----------------------------------------------------------------------------*/
DWORD WINAPI ProcessTCP_IO(LPVOID lpParameter)
{
	DWORD Index;
	DWORD Flags;
	LPSOCKET_INFORMATION SI;
	DWORD BytesTransferred;
	DWORD i;
	DWORD RecvBytes = 0;
	DWORD BytesSent = 0;
	DWORD UDPthreadID;
	string ReceivedData;
	char temp[TEMP_BUFSIZE];
	HWND hwnd = (HWND)lpParameter;
	UDP_INFO *udpInfo;

	udpInfo = (UDP_INFO*)malloc(sizeof(UDP_INFO)); 

	udpInfo->hwnd = hwnd;
  
   // Process asynchronous WSASend, WSARecv requests.

	while(TRUE)
	{
		if ((Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents failed %d\n", WSAGetLastError());
			return 0;
		} 

		// If the event triggered was zero then a connection attempt was made
		// on our listening socket.
 
		if ((Index - WSA_WAIT_EVENT_0) == 0)
		{
			WSAResetEvent(EventArray[0]);
			continue;
		}

		SI = SocketArray[Index - WSA_WAIT_EVENT_0];
		//Control socket is the first socket in the array after the listen socket.
		ControlSocket = SocketArray[1]->Socket;
	
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
		
		if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred,
			FALSE, &Flags) == FALSE || BytesTransferred == 0)
		{
			sprintf(temp, "Closing socket %d", SI->Socket);
			infoVector.push_back(temp);	
			PrintIOLog(infoVector, hwnd);
			
			TotalTCPBytes = 0;

			if (closesocket(SI->Socket) == SOCKET_ERROR)
			{
				printf("closesocket() failed with error %d\n", WSAGetLastError());
			}

			TerminateThread(hThrdUDP, 0);

			GlobalFree(SI);
			WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

			// Cleanup SocketArray and EventArray by removing the socket event handle
			// and socket information structure if they are not at the end of the
			// arrays.

			EnterCriticalSection(&CriticalSection);

			if ((Index - WSA_WAIT_EVENT_0) + 1 != EventTotal)
				for (i = Index - WSA_WAIT_EVENT_0; i < EventTotal; i++)
				{
					EventArray[i] = EventArray[i + 1];
					SocketArray[i] = SocketArray[i + 1];
				}

			EventTotal--;

			LeaveCriticalSection(&CriticalSection);

			continue;
		}

		/*BytesRECV is 0, read socket call finished check received data.*/
		if (SI->BytesRECV == 0)
		{
			SI->BytesRECV = BytesTransferred;
	
			SI->BytesSEND = 0;

			//Don't count bytes sent on the control channel
			if (SI->Socket != ControlSocket)
				TotalTCPBytes += SI->BytesRECV;
	
			ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
			SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

			ReceivedData += SI->Buffer;

			// Check for control characters
			if (atoi(SI->Buffer) == EOT) //End of Transmission
			{
				sprintf(temp, "Total Received TCP bytes: %d", TotalTCPBytes);
				infoVector.push_back(temp);

				sprintf(temp, "Total Received UDP bytes: %d", TotalUDPBytes);
				infoVector.push_back(temp);

				if (Mode == TCP_MODE)
				{
					sprintf(temp, "%d", TotalTCPBytes);
					SI->DataBuf.buf = temp;
					SI->DataBuf.len = strlen(temp);
				}
				else if (Mode == UDP_MODE)
				{
					sprintf(temp, "%d", TotalUDPBytes);
					SI->DataBuf.buf = temp;
					SI->DataBuf.len = strlen(temp);
				}
	
				BytesSent = WriteToSocket(&SI->Socket, &SI->DataBuf, &SI->Overlapped);

				TotalTCPBytes = 0;
				TotalUDPBytes = 0;
				BytesSent = 0;
				PrintIOLog(infoVector, hwnd);
			}
			else if(strcmp(SI->Buffer, "udp") == 0) //UDP mode start the UDP IO thread.
			{
				Mode = UDP_MODE;
				infoVector.clear();

				if ((hThrdUDP = CreateThread(NULL, 0, ProcessUDP_IO, (LPVOID)udpInfo, 0, &UDPthreadID)) == NULL)
				{
					printf("CreateThread failed with error %d\n", GetLastError());
					return 0;
				} 
			} 
			else if (strcmp(SI->Buffer, "tcp") == 0) //TCP mode
			{
				Mode = TCP_MODE;
				infoVector.clear();
			}
			
		}	

		Flags = 0;
		ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
		SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

		SI->DataBuf.len = DATA_BUFSIZE;
		SI->DataBuf.buf = SI->Buffer;
		SI->BytesRECV = 0;
		RecvBytes = ReadSocket(&SI->Socket, &SI->DataBuf, Flags, &SI->Overlapped);
	}
}

/*------------------------------------------------------------------------------
--	FUNCTION: ProcessUDP_IO()
--
--	PURPOSE:		Processes IO on the UDP channel
--
--	PARAMETERS:
--		LPVOID lpParameter		-Parameter for the thread, Will be the handle 
--								 to the window where results are to be displayed
--
--	DESIGNERS:		Filip Gutica 
--
--	PROGRAMMER:		Filip Gutica 
/*-----------------------------------------------------------------------------*/
DWORD WINAPI ProcessUDP_IO(LPVOID lpParameter)
{
	DWORD Index;
	DWORD BytesTransferred = 0;
	DWORD Flags = 0;
	DWORD RecvBytes;
	WSABUF DataBuf;
	WSAOVERLAPPED ol;
	char temp[BUFFER_SIZE];
	char Buffer[BUFFER_SIZE];
	UDP_INFO *UDPinfo = (UDP_INFO*) lpParameter;
	DWORD BytesRECV = 0;

	while (TRUE)
	{
		DataBuf.buf = Buffer;
		DataBuf.len = DATA_BUFSIZE;
		ZeroMemory(&ol, sizeof(WSAOVERLAPPED));
		ReadSocket(&UDPSock, &DataBuf, Flags, &ol);
	
		if ((Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			100, FALSE)) == WSA_WAIT_FAILED)
		{
			sprintf(temp, "WSAWaitForMultipleEvents failed %d\n", WSAGetLastError());
			infoVector.push_back(temp);
			return 0;
		} 

		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		if (WSAGetOverlappedResult(UDPSock, &ol, &BytesTransferred, FALSE, &Flags) == FALSE)
		{
			break;
		}

		TotalUDPBytes += BytesTransferred;

		if (BytesRECV == 0)
		{
			//infoVector.push_back(DataBuf.buf);
			//PrintIOLog(infoVector, UDPinfo->hwnd);
			BytesRECV = BytesTransferred;
		}

		DataBuf.buf = Buffer;
		DataBuf.len = DATA_BUFSIZE;
		BytesRECV = 0;
		ZeroMemory(&ol, sizeof(WSAOVERLAPPED));
		ReadSocket(&UDPSock, &DataBuf, Flags, &ol);

		if (WSASetEvent(EventArray[0]) == FALSE)
		{
			printf("WSASetEvent failed with error %d\n", WSAGetLastError());
			return 0;
		}
	}
}


/*------------------------------------------------------------------------------
--	FUNCTION: PrintIOLog()
--
--	PURPOSE:		Helper function to display results to the screen
--
--	PARAMETERS:
--		vector v	-Vector of log info to be displayed
--		HWND h		-Handle to the window where data is to be displayed
--
--	DESIGNERS:		Filip Gutica 
--
--	PROGRAMMER:		Filip Gutica
/*-----------------------------------------------------------------------------*/
void PrintIOLog(vector<string> v , HWND h)
{
	string result = "";
	if (!v.empty())
	{
		for (int j = 0; j < v.size(); j++)
		{
			
			result += v.at(j);
			result+= "\r\n";

		}

		SetWindowText(h, result.c_str());
		//v.clear();
	}

}

/*------------------------------------------------------------------------------
--	FUNCTION: WriteToSocket()
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
DWORD WriteToSocket(SOCKET *sock, WSABUF *buf, WSAOVERLAPPED *ol)
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
--	FUNCTION: ReadSocket()
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
DWORD ReadSocket(SOCKET *sock, WSABUF *buf, DWORD fl,  WSAOVERLAPPED *ol)
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

/*------------------------------------------------------------------------------
--	FUNCTION: StopServer()
--
--	PURPOSE:		Stop the server do any necesarry cleanup
--
--	PARAMETERS:
--			void
--
--	Return:
			void
--
--	DESIGNERS:		Filip Gutica 
--
--	PROGRAMMER:		Filip Gutica 
--
/*-----------------------------------------------------------------------------*/
void StopServer()
{
	if (closesocket(UDPSock) == SOCKET_ERROR)
		return;
	if (closesocket(ListenSocket) == SOCKET_ERROR)
		return;
	if (closesocket(AcceptSocket) == SOCKET_ERROR)
		return;

	TerminateThread(hThrdListen, 0);
	TerminateThread(hThrdIO, 0);
	WSACleanup();
}

