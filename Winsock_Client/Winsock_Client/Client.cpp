#include<winsock2.h>
#include<ws2tcpip.h>
#include<stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFFLENGTH 512
int main()
{
	int iResult;
	WSAData wsData;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD(2,2);
	iResult = WSAStartup(wVersionRequested,&wsData);
	if(iResult != 0)
	{
		printf("init failed");
		return 1;
	}
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	ZeroMemory(&hints,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	char *ipaddr = "127.0.0.1";
	iResult = getaddrinfo(ipaddr,DEFAULT_PORT,&hints,&result);
	if(iResult != 0)
	{
		printf("Getting address information failed");
		return 1;
	}
	SOCKET connectSocket = INVALID_SOCKET;

	ptr = result;
	connectSocket = socket(ptr->ai_family,ptr->ai_socktype,ptr->ai_protocol);
	if(connectSocket == INVALID_SOCKET)
	{
		printf("Socket is invalid");
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	iResult = connect(connectSocket,ptr->ai_addr,(int)ptr->ai_addrlen);
	if(iResult == SOCKET_ERROR)
	{
		printf("Innvalid Socket");
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
	}
	if(connectSocket == INVALID_SOCKET)
	{
		printf("Connection failed");
		WSACleanup();
		return 1;
	}
	// To send the message
	char *message = "This is a test";
	char receiveBuffer[DEFAULT_BUFFLENGTH];
	iResult = send(connectSocket, message,(int)strlen(message),0);
	if(iResult == SOCKET_ERROR)
	{
		printf("Soceket Failed");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	printf("Sent %ld\n",iResult);
	iResult = shutdown(connectSocket, SD_SEND);
	if(iResult == SOCKET_ERROR)
	{
		printf("Shutdown Failed");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	do
	{
		iResult = recv(connectSocket, receiveBuffer,DEFAULT_BUFFLENGTH,0);
		if(iResult > 0)
			printf("Received in: %ld",iResult);
		else if(iResult == 0)
			printf("connecion closed");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	}while (iResult > 0);
	iResult = shutdown(connectSocket, SD_SEND);
	if(iResult == SOCKET_ERROR)
	{
		printf("Shutdown failed due to: %d\n",WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}
	
	return 0;
}