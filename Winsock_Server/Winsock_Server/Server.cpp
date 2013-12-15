#include<WinSock2.h>
#include<WS2tcpip.h>
#include<tchar.h>
#include<strsafe.h>
#include<stdio.h>
#include<iostream>
#include<vector>
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFF_LEN 512
#define MAX_THREADS 100

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

HANDLE hThrds[MAX_THREADS];
HANDLE hListener;
DWORD threadId;

WORD wVersionSelected;
struct addrinfo *result = NULL, *ptr = NULL, hints;

void ClientThread(SOCKET clientSockThr)
{
	std::cout<<"Processing thread "<<std::endl;
	char rcvbuff[DEFAULT_BUFF_LEN];
	int bufflen = DEFAULT_BUFF_LEN;
	int iSendResult;
	int iResult;
	do{
		iResult= recv(clientSockThr, rcvbuff,bufflen,0);
		if(iResult > 0)
		{
			printf("Bytes received: %d\n",iResult);
			// Echo the message
			iSendResult = send(clientSockThr,rcvbuff,iResult,0);
			if(iSendResult == SOCKET_ERROR)
			{
				printf("Echo failed %d\n",WSAGetLastError());
				closesocket(clientSockThr);
				WSACleanup();
				return;
			}
			printf("Bytes returned %d\n",iSendResult);
		}else if(iResult == 0)
			printf("Connection Closing");
		else
		{
			printf("Reception falied\n");
			closesocket(clientSockThr);
			WSACleanup();
			return;
		}
	}while(iResult > 0);
	iResult = shutdown(clientSockThr,SD_SEND);
	if(iResult == SOCKET_ERROR)
	{
		printf("Something happened while shutting down the socket");
		closesocket(clientSockThr);
		WSACleanup();
		return;
	}
	closesocket(clientSockThr);
}
void SocketListener(SOCKET pListenerSock)
{
	int index = 0;
	while(listen(pListenerSock, SOMAXCONN) != SOCKET_ERROR)
	{
		
		SOCKET clientSock =  INVALID_SOCKET;
		clientSock = accept(pListenerSock,NULL,NULL);
		if(clientSock != INVALID_SOCKET)
		{
			std::cout<<"Starting a new Listening thread\n";
			hThrds[index] = CreateThread(0,0,(LPTHREAD_START_ROUTINE) ClientThread,(LPVOID)clientSock,	0,&threadId );
			
			if(hThrds[index])
			{

				std::cout<<"Thread launched."<<std::endl;
			}
		}	

		else
		{
			printf("Accept fialed %d\n",WSAGetLastError());
			closesocket(pListenerSock);
			WSACleanup();
			return;
		}
	}

}
int initSockets(SOCKET &pListenerSock)
{
	int iResult;
	pListenerSock = INVALID_SOCKET;
	wVersionSelected = MAKEWORD(2,2);
	WSAData wsaData;
	iResult = WSAStartup(wVersionSelected,&wsaData);
	if(iResult != 0)
	{
		printf("Failed to start up server. Returned code: %d",iResult);
		return 1;
	}
	
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
	pListenerSock = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if(pListenerSock == INVALID_SOCKET)
	{
		printf("Error in creating the socket %d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	iResult = bind(pListenerSock, result->ai_addr, (int)result->ai_addrlen);
	if(iResult == SOCKET_ERROR)
	{
		printf("Some crap happened while binding the socket: %d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);
	return 0;
}
int main()
{
	SOCKET listenerSock;
	int iResult = initSockets(listenerSock);
	if(iResult != 0)
	{
		printf("Init failed!!!");
		return 1;
	}
	// Start the listener thread
	hListener = CreateThread(0,0,(LPTHREAD_START_ROUTINE) SocketListener,(LPVOID)listenerSock,0,&threadId);
	if(hListener)
	{
		printf("Started a new listener thread");
	}
	
	WaitForMultipleObjects(MAX_THREADS,hThrds,TRUE, INFINITE);
	for (int k=0; k<MAX_THREADS; k++)
	{
		CloseHandle(hThrds[k]);
	}
	WaitForSingleObject(hListener,INFINITE);
	CloseHandle(hListener);

	return 0;
}
