// SelectTCPServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
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
SOCKETINFO *SocketInfoArray[FD_SETSIZE];
BOOL AddSocketInfo(SOCKET sock) {
	if (nTotalSockets >= FD_SETSIZE) return FALSE;
	SOCKETINFO *ptr = new SOCKETINFO;
	if (ptr == NULL) return FALSE;
	ptr->sock = sock;	
	ZeroMemory(ptr->buf, BUFSIZE + 1);
	ptr->recvbytes = 0; ptr->sendbytes = 0;
	SocketInfoArray[nTotalSockets++] = ptr;
}
void RemoveSocketInfo(int nIndex) {
	SOCKETINFO *ptr = SocketInfoArray[nIndex];
	closesocket(ptr->sock);
	delete ptr;	ptr = NULL;
	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] =
			SocketInfoArray[nTotalSockets - 1];
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
	u_long on = 1;
	retval = ioctlsocket(listen_sock, FIONBIO, &on);
	FD_SET rset, wset; 
	while (1) {
		FD_ZERO(&rset); FD_ZERO(&wset);
		FD_SET(listen_sock, &rset);
		for (int i = 0; i < nTotalSockets; i++) {
			SOCKETINFO *ptr = SocketInfoArray[i];
			if (ptr->recvbytes > ptr->sendbytes) {
				FD_SET(ptr->sock, &wset);
			}
			else {
				FD_SET(ptr->sock, &rset);
			}
		}
		retval = select(0, &rset, &wset, NULL, NULL);
		if (FD_ISSET(listen_sock, &rset)) {
			SOCKADDR_IN clientaddr = { 0 };
			int addrlen = sizeof(clientaddr);
			SOCKET client_sock = accept(listen_sock,
				(SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock != INVALID_SOCKET) {
				AddSocketInfo(client_sock);
			}
		}
		for (int i = 0; i < nTotalSockets; i++) {
			SOCKETINFO *ptr = SocketInfoArray[i];
			if (FD_ISSET(ptr->sock, &rset)) {
				retval = recv(ptr->sock, ptr->buf,BUFSIZE, 0);
				if (retval == SOCKET_ERROR || retval == 0) {
					RemoveSocketInfo(i); continue;
				}
				ptr->recvbytes = retval;
				ptr->buf[retval] = '\0';
				printf("%s\n", ptr->buf);
			}
			if (FD_ISSET(ptr->sock, &wset)) {
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
		}
	}
	///////////////////////////////////////////////////
	closesocket(listen_sock);
	WSACleanup();
    return 0;
}

