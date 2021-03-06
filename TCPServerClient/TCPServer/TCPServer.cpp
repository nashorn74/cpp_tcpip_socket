// TCPServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//
#include "stdafx.h"
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 512


int main()
{
	int retval;
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	//---------------------------------------------
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) {
		MessageBox(NULL, "socket() error", "error", MB_OK);
		exit(1);
	}
	SOCKADDR_IN serveraddr = {0};
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr,
		sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		MessageBox(NULL, "bind() error", "error", MB_OK);
		exit(1);
	}
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		MessageBox(NULL, "listen() error", "error", MB_OK);
		exit(1);
	}
	printf("----------------------------");
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	char buf[BUFSIZE + 1];
	while (1) {
		client_sock = accept(listen_sock,
			(SOCKADDR*)&clientaddr, &addrlen);
		printf("\n Client IP=%s PORT=%d\n",
			inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port));
		while (1) {
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				printf("recv() error"); break;
			}
			else if (retval == 0) break;
			buf[retval] = '\0';
			printf("%s\n", buf);
			retval = send(client_sock, buf, retval, 0);
			if (retval == SOCKET_ERROR) {
				printf("send() error"); break;
			}
		}
		closesocket(client_sock);
	}

	closesocket(listen_sock);
	//---------------------------------------------
	WSACleanup();
    return 0;
}

