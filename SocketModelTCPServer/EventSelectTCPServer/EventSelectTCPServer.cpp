// EventSelectTCPServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#define SERVERPORT 9000
#define BUFSIZE 512
struct SOCKETINFO {
	SOCKET sock; char buf[BUFSIZE + 1];
	int recvbytes; int sendbytes;
};
int nTotalSockets = 0;
SOCKETINFO *SocketInfoArray[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
BOOL AddSocketInfo(SOCKET sock) {
	if (nTotalSockets >= FD_SETSIZE) return FALSE;
	SOCKETINFO *ptr = new SOCKETINFO;
	if (ptr == NULL) return FALSE;
	ptr->sock = sock;
	ZeroMemory(ptr->buf, BUFSIZE + 1);
	ptr->recvbytes = 0; ptr->sendbytes = 0;
	SocketInfoArray[nTotalSockets] = ptr;//****
	WSAEVENT hEvent = WSACreateEvent();//****
	EventArray[nTotalSockets++] = hEvent;//****
}
void RemoveSocketInfo(int nIndex) {
	SOCKETINFO *ptr = SocketInfoArray[nIndex];
	closesocket(ptr->sock);
	delete ptr;	ptr = NULL;
	WSACloseEvent(EventArray[nIndex]);//****
	if (nIndex != (nTotalSockets - 1)) {
		SocketInfoArray[nIndex] =
			SocketInfoArray[nTotalSockets - 1];
		EventArray[nIndex] = //****
			EventArray[nTotalSockets - 1];//****
	}
	--nTotalSockets;
}
int main()
{
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN serveraddr = { 0 };
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = bind(listen_sock, (SOCKADDR*)&serveraddr,
		sizeof(serveraddr));
	retval = listen(listen_sock, SOMAXCONN);
	///////////////////////////////////////////////////
	AddSocketInfo(listen_sock);
	retval = WSAEventSelect(listen_sock,
		EventArray[0], FD_ACCEPT | FD_CLOSE);
	while (1) {
		int i = WSAWaitForMultipleEvents(nTotalSockets,
			EventArray, FALSE, WSA_INFINITE, FALSE);
		printf("---------\n");
		if (i == WSA_WAIT_FAILED) continue;
		i -= WSA_WAIT_EVENT_0;//Index값 구하기
		printf("%d\n", i);
		WSANETWORKEVENTS NetworkEvents;
		retval = WSAEnumNetworkEvents(
			SocketInfoArray[i]->sock, EventArray[i],
			&NetworkEvents);
		SOCKETINFO *ptr = SocketInfoArray[i];
		if (NetworkEvents.lNetworkEvents & FD_ACCEPT) {
			SOCKADDR_IN clientaddr = { 0 };
			int addrlen = sizeof(clientaddr);
			SOCKET client_sock = accept(ptr->sock,
				(SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock != INVALID_SOCKET) {
				AddSocketInfo(client_sock);
				WSAEventSelect(client_sock,
					EventArray[nTotalSockets - 1],
					FD_READ | FD_WRITE | FD_CLOSE);
				printf("accept");
			}
		}
		if (NetworkEvents.lNetworkEvents & FD_READ ||
			NetworkEvents.lNetworkEvents & FD_WRITE) {
			printf("recv----");
			retval = recv(ptr->sock, ptr->buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				RemoveSocketInfo(i); continue;
			}
			ptr->recvbytes = retval;
			ptr->buf[retval] = '\0';
			printf("%s\n", ptr->buf);
			printf("send----");
			retval = send(ptr->sock, ptr->buf + ptr->sendbytes,
				ptr->recvbytes - ptr->sendbytes, 0);
			if (retval == SOCKET_ERROR) {
				RemoveSocketInfo(i); continue;
			}
			ptr->sendbytes += retval;
			if (ptr->recvbytes == ptr->sendbytes) {
				ptr->recvbytes = ptr->sendbytes = 0;
			}
		}
		if (NetworkEvents.lNetworkEvents & FD_CLOSE) {
			RemoveSocketInfo(i);
		}
	}
	///////////////////////////////////////////////////
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
