// BroadcastSender.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#define REMOTEIP "255.255.255.255"
#define REMOTEPORT 9000
#define BUFSIZE 512

int main()
{
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	BOOL bEnable = TRUE;
	int retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
		(char*)&bEnable, sizeof(bEnable));
	SOCKADDR_IN remoteaddr = { 0 };
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr(REMOTEIP);
	remoteaddr.sin_port = htons(REMOTEPORT);
	////////////////////////////////////////////////
	char buf[BUFSIZE + 1];
	while (1) {
		printf("\n Input Data : ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL) break;
		int len = strlen(buf);
		if (buf[len - 1] == '\n') buf[len - 1] = '\0';
		if (strlen(buf) == 0) break;
		int retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR*)&remoteaddr, sizeof(remoteaddr));
	}
	////////////////////////////////////////////////
	closesocket(sock);
	WSACleanup();
	return 0;
}