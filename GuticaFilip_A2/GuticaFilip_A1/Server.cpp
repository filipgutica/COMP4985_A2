#include "Server.h"



DWORD EventTotal = 0;
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION CriticalSection;   
SOCKET ListenSocket, AcceptSocket, ControlSocket;
HANDLE hThrdIO, hThrdListen;
vector<string> infoVector;


void StartServer(HWND h)
{
	WSADATA wsaData;
	SOCKADDR_IN InternetAddr;
	DWORD Flags;
	DWORD IOThreadId;
	DWORD ListenThreadId;
	DWORD RecvBytes;
	INT Ret;

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

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);

	if (bind(ListenSocket, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
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

      // Fill in the details of our accepted socket.
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


DWORD WINAPI ProcessTCP_IO(LPVOID lpParameter)
{
	DWORD Index;
	DWORD Flags;
	LPSOCKET_INFORMATION SI;
	DWORD BytesTransferred;
	DWORD i;
	DWORD RecvBytes = 0;
	DWORD BytesSent = 0;
	char temp[TEMP_BUFSIZE];
	HWND hwnd = (HWND)lpParameter;
	int TotalBytes;
	UDP_INFO udpInfo;
	udpInfo.hwnd = hwnd;
	
  
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
		ControlSocket = SocketArray[1]->Socket;
	
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
		
		if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred,
			FALSE, &Flags) == FALSE || BytesTransferred == 0)
		{
			sprintf(temp, "Closing socket %d", SI->Socket);
			infoVector.push_back(temp);	
			
			infoVector.clear();
			TotalBytes = 0;

			if (closesocket(SI->Socket) == SOCKET_ERROR)
			{
				printf("closesocket() failed with error %d\n", WSAGetLastError());
			}

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

		// Check to see if the BytesRECV field equals zero. If this is so, then
		// this means a WSARecv call just completed so update the BytesRECV field
		// with the BytesTransferred value from the completed WSARecv() call.
		if (SI->BytesRECV == 0)
		{
			SI->BytesRECV = BytesTransferred;
	
			SI->BytesSEND = 0;

			//Don't count bytes sent on the control channel
			if (SI->Socket != ControlSocket)
				TotalBytes += (SI->BytesRECV);

			// Check for control characters
			ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
			SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
			
			if (atoi(SI->Buffer) == EOT)
			{
				sprintf(temp, "Total Received bytes: %d", TotalBytes);
				infoVector.push_back(temp);

				sprintf(temp, "%d", TotalBytes);
				SI->DataBuf.buf = temp;
				SI->DataBuf.len = strlen(temp);

				//Keep writing till all bytes sent
				
				while(BytesSent != SI->DataBuf.len)
					BytesSent = WriteToSocket(SI->Socket, SI->DataBuf, SI->Overlapped);

				BytesSent = 0;
				
			}
			else if(udpInfo.ip = inet_addr(SI->Buffer) != INADDR_NONE && strstr(SI->Buffer, ".") != NULL)
			{
				MessageBox(NULL, "got IP", "", MB_OK);
			}
			else
			{
				sprintf(temp, "%d", ACK);
				SI->DataBuf.buf = temp;
				SI->DataBuf.len = strlen(temp);

				while(BytesSent != SI->DataBuf.len)
					BytesSent = WriteToSocket(SI->Socket, SI->DataBuf, SI->Overlapped);

				BytesSent = 0;
			}
			
		}
		else
		{
			
			SI->BytesSEND += BytesTransferred;
		}

		SI->BytesRECV = 0;

		// Now that there are no more bytes to send post another WSARecv() request.

		Flags = 0;
		ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
		SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

		SI->DataBuf.len = DATA_BUFSIZE;
		SI->DataBuf.buf = SI->Buffer;

		RecvBytes = ReadSocket(&SI->Socket, &SI->DataBuf, Flags, &SI->Overlapped);

		PrintIOLog(infoVector, hwnd);
	}
	
}

DWORD WINAPI ProcessUDP_IO(LPVOID lpParameter)
{
	DWORD Index;
	DWORD Flags;
	LPSOCKET_INFORMATION SI;
	DWORD BytesTransferred;
	DWORD i;
	DWORD RecvBytes = 0;
	DWORD BytesSent = 0;
	char temp[TEMP_BUFSIZE];
	HWND hwnd = (HWND)lpParameter;
	int TotalBytes;
  
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
		ControlSocket = SocketArray[1]->Socket;
	
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
		
		if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred,
			FALSE, &Flags) == FALSE || BytesTransferred == 0)
		{
			sprintf(temp, "Closing socket %d", SI->Socket);
			infoVector.push_back(temp);	
			
			infoVector.clear();
			TotalBytes = 0;

			if (closesocket(SI->Socket) == SOCKET_ERROR)
			{
				printf("closesocket() failed with error %d\n", WSAGetLastError());
			}

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

		// Check to see if the BytesRECV field equals zero. If this is so, then
		// this means a WSARecv call just completed so update the BytesRECV field
		// with the BytesTransferred value from the completed WSARecv() call.
		if (SI->BytesRECV == 0)
		{
			SI->BytesRECV = BytesTransferred;
	
			SI->BytesSEND = 0;

			//Don't count bytes sent on the control channel
			if (SI->Socket != ControlSocket)
				TotalBytes += (SI->BytesRECV);

			// Check for control characters
			ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
			SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
			
			if (atoi(SI->Buffer) == EOT)
			{
				sprintf(temp, "Total Received bytes: %d", TotalBytes);
				infoVector.push_back(temp);

				sprintf(temp, "%d", TotalBytes);
				SI->DataBuf.buf = temp;
				SI->DataBuf.len = strlen(temp);

				//Keep writing till all bytes sent
				
				while(BytesSent != SI->DataBuf.len)
					BytesSent = WriteToSocket(SI->Socket, SI->DataBuf, SI->Overlapped);

				BytesSent = 0;
				
			}
			else
			{
				sprintf(temp, "%d", ACK);
				SI->DataBuf.buf = temp;
				SI->DataBuf.len = strlen(temp);

				while(BytesSent != SI->DataBuf.len)
					BytesSent = WriteToSocket(SI->Socket, SI->DataBuf, SI->Overlapped);

				BytesSent = 0;
			}
			
		}
		else
		{
			
			SI->BytesSEND += BytesTransferred;
		}

		SI->BytesRECV = 0;

		// Now that there are no more bytes to send post another WSARecv() request.

		Flags = 0;
		ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
		SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

		SI->DataBuf.len = DATA_BUFSIZE;
		SI->DataBuf.buf = SI->Buffer;

		RecvBytes = ReadSocket(&SI->Socket, &SI->DataBuf, Flags, &SI->Overlapped);

		PrintIOLog(infoVector, hwnd);
	}
	
}

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


DWORD WriteToSocket(SOCKET sock, WSABUF buf, WSAOVERLAPPED ol)
{
	DWORD sb;
	char temp[64];

	if (WSASend(sock, &buf, 1, &sb, 0,
		&ol, NULL) == SOCKET_ERROR)
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

DWORD ReadSocket(SOCKET *sock, WSABUF *buf, DWORD fl,  WSAOVERLAPPED *ol)
{
	DWORD rb;
	char temp[64];

	if (WSARecv(*sock, buf, 1, &rb, &fl,
			ol, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{

			sprintf(temp, "WSASend() failed with error %d", WSAGetLastError());
			MessageBox(NULL, temp, "", MB_OK);
			return 0;
		}
			
	}

	return rb;
}

void StopServer()
{
	if (closesocket(ListenSocket) == SOCKET_ERROR)
		return;
	if (closesocket(AcceptSocket) == SOCKET_ERROR)
		return;

	TerminateThread(hThrdListen, 0);
	TerminateThread(hThrdIO, 0);
}

