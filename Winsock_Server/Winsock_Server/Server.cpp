#include<WinSock2.h>
#include<WS2tcpip.h>
#include<stdio.h>
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFF_LEN 512

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
int main()
{
	int iResult;
	WSAData wsaData;
	WORD wVersionSelected;
	SOCKET listenerSock = INVALID_SOCKET;
	SOCKET clientSock = INVALID_SOCKET;
	wVersionSelected = MAKEWORD(2,2);
	iResult = WSAStartup(wVersionSelected,&wsaData);
	if(iResult != 0)
	{
		printf("Failed to start up server. Returned code: %d",iResult);
		return 1;
	}
	struct addrinfo *result = NULL, *ptr = NULL,hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	iResult = getaddrinfo(NULL,DEFAULT_PORT,&hints,&result);
	if(iResult != 0)
	{
		printf("Some crap happened while gettingaddress info");
		WSACleanup();
		return 1;
	}
	listenerSock = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if(listenerSock == INVALID_SOCKET)
	{
		printf("Error in creating the socket %d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	iResult = bind(listenerSock, result->ai_addr, (int)result->ai_addrlen);
	if(iResult == SOCKET_ERROR)
	{
		printf("Some crap happened while binding the socket: %d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);
	if(listen(listenerSock, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("something went wrong while listening: %d\n",WSAGetLastError());
		closesocket(listenerSock);
		WSACleanup();
		return 1;
	}
	// Code to accept the client socket
	clientSock  = accept(listenerSock,NULL,NULL);
	if(clientSock == INVALID_SOCKET)
	{
		printf("Accept fialed %d\n",WSAGetLastError());
		closesocket(listenerSock);
		WSACleanup();
		return 1;
	}
	closesocket(listenerSock);
	char rcvbuff[DEFAULT_BUFF_LEN];
	int bufflen = DEFAULT_BUFF_LEN;
	int iSendResult;
	do{
		iResult= recv(clientSock, rcvbuff,bufflen,0);
		if(iResult > 0)
		{
			printf("Bytes received: %d\n",iResult);
			// Echo the message
			iSendResult = send(clientSock,rcvbuff,iResult,0);
			if(iSendResult == SOCKET_ERROR)
			{
				printf("Echo failed %d\n",WSAGetLastError());
				closesocket(clientSock);
				WSACleanup();
				return 1;
			}
			printf("Bytes returned %d\n",iSendResult);
		}else if(iResult == 0)
			printf("Connection Closing");
		else
		{
			printf("Reception falied\n");
			closesocket(clientSock);
			WSACleanup();
			return 1;
		}
	}while(iResult > 0);
	// Shut down the socket
	iResult = shutdown(clientSock,SD_SEND);
	if(iResult == SOCKET_ERROR)
	{
		printf("Something happened while shutting down the socket");
		closesocket(clientSock);
		WSACleanup();
		return 1;
	}
	closesocket(clientSock);
	WSACleanup();
	return 0;
}