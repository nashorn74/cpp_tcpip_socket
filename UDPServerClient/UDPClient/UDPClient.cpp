// UDPClient.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
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
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN serveraddr = { 0 };
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	////////////////////////////////////////////////
	SOCKADDR_IN peeraddr = { 0 };
	int addrlen = sizeof(peeraddr);
	char buf[BUFSIZE + 1];
	while (1) {
		printf("\n Input Data : ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL) break;
		int len = strlen(buf);
		if (buf[len - 1] == '\n') buf[len - 1] = '\0';
		if (strlen(buf) == 0) break;
		int retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR*)&serveraddr, sizeof(serveraddr));
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&peeraddr, &addrlen);
		if (memcmp(&peeraddr, &serveraddr, addrlen)) {
			printf("wrong data!"); continue;
		}
		buf[retval] = '\0';
		printf("%s\n", buf);
	}
	////////////////////////////////////////////////
	closesocket(sock);
	WSACleanup();
    return 0;
}

