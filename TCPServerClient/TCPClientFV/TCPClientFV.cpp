// TCPClientFV.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

int main()
{
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		MessageBox(NULL, "socket() Error", "Error", MB_OK);
		exit(1);
	}
	SOCKADDR_IN serveraddr = { 0 };
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = connect(sock, (SOCKADDR*)&serveraddr,
		sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		MessageBox(NULL, "connect() Error", "Error", MB_OK);
		exit(1);
	}

	char buf[BUFSIZE];
	char *testdata[] = {
		"안녕하세요", "반가워요",
		"오늘따라 할 이야기가 많을 것 같네요.",
		"저도 그렇네요"
	};
	int len;
	for (int i = 0; i < 4; i++) {
		len = strlen(testdata[i]);
		strncpy(buf, testdata[i], len);

		retval = send(sock, (char*)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			printf("send() error\n"); break;
		}
		retval = send(sock, buf, len, 0);
		if (retval == SOCKET_ERROR) {
			printf("send() error\n"); break;
		}
		printf("%d+%d bytes send...\n", sizeof(int), retval);
	}

	closesocket(sock);
	WSACleanup();
    return 0;
}

