#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(void)
{
	
	WSADATA wsaData; //Структура WSADATA содержит информацию о реализации Windows Sockets.
	int iResult;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	//Структура addrinfo используется функцией getaddrinfo для хранения информации об адресе хоста.
	struct addrinfo* result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	

	// Initialize Winsock
	//
//Для чего используется MAKEWORD?
//Я встретил этот макрос MAKEWORD(2,2) в куске учебного кода. Я прочитал в MSDN, что он "создает значение WORD, объединяя указанные значения".
//Единственный реальный пример этого макроса - это Инициализация Winsock, чтобы генерировать слово для версии, ожидаемое WSAStartup.
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}
	
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //Семейство адресов AF_INET - это семейство адресов для IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	
	/*
	Установка флага AI_PASSIVE указывает, что вызывающая сторона намерена
	использовать возвращенную структуру адреса сокета при вызове функции связывания . Когда установлен флаг AI_PASSIVE,
	а pNodeName является указателем NULL , часть IP-адреса структуры адреса сокета устанавливается на INADDR_ANY для адресов
	 IPv4 и IN6ADDR_ANY_INIT для адресов IPv6.
Когда флаг AI_PASSIVE не установлен, возвращаемая структура адреса сокета готова для вызова функции connect для протокола с
установлением соединения или готова для вызова функций connect , sendto или send для протокола без установления соединения.
Если pNodeName параметром является значение NULL указателя в этом случае часть адреса IP - сокет структуры адреса установлена на адрес обратной петли.
*/
// Resolve (разрешать) the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	
	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	
	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	
	freeaddrinfo(result);
	
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}


	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//printf("Hello");
	printf("Successfully connected to the client and ready to delete file\nEnter the path: ");
	char sendbuf[512];
	scanf("%s", sendbuf);
	sendbuf[strlen(sendbuf)] = '\0';
	//Функция отправки отправляет данные в подключенный сокет.
	iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
	}
	memset(recvbuf, '\0', recvbuflen);
	//Функция recv получает данные из подключенного сокета или из подключенного сокета без установления соединения.
	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (iResult != SOCKET_ERROR)
	{
		printf("%s", recvbuf);
	}
	//!!!!!!
	//это надо?
	/*
		closesocket(ClientSocket);
		WSACleanup();
	*/
	return 0;
}
