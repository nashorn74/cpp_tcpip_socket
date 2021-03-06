// TCPServerFV.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 512

int recvn(SOCKET s, char *buf, int len, int flags) {
	int received; char *ptr = buf; int left = len;
	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR) return SOCKET_ERROR;
		else if (received == 0) break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}

int main()
{
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) {
		MessageBox(NULL, "socket() Error", "Error", MB_OK);
		exit(1);
	}
	SOCKADDR_IN serveraddr = { 0 };
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = bind(listen_sock, (SOCKADDR*)&serveraddr,
		sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		MessageBox(NULL, "bind() Error", "Error", MB_OK);
		exit(1);
	}
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		MessageBox(NULL, "listen() Error", "Error", MB_OK);
		exit(1);
	}
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	char buf[BUFSIZE + 1];
	int len;
	while (1) {
		client_sock = accept(listen_sock,
			(SOCKADDR*)&clientaddr, &addrlen);
		while (1) {
			retval = recvn(client_sock, (char*)&len,
				sizeof(int), 0);//고정길이데이터 수신
			if (retval == SOCKET_ERROR) {
				printf("recv() error\n"); break;
			}
			else if (retval == 0) break;
			//가변길이 데이터 수신
			retval = recvn(client_sock, buf, len, 0);
			if (retval == SOCKET_ERROR) {
				printf("recv() error\n"); break;
			}
			else if (retval == 0) break;
			buf[retval] = '\0';
			printf("%s\n", buf);
		}
		closesocket(client_sock);
	}
	closesocket(listen_sock);
	WSACleanup();
    return 0;
}

