// MulticastReceiver.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#define MULTICASTIP "146.11.76.31"
#define LOCALPORT 9000
#define BUFSIZE 512
int main()
{
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	BOOL optval = TRUE;
	int retval = setsockopt(sock, SOL_SOCKET,
		SO_REUSEADDR, (char*)&optval, sizeof(optval));
	SOCKADDR_IN serveraddr = { 0 };
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(LOCALPORT);
	retval = bind(sock, (SOCKADDR*)&serveraddr,
		sizeof(serveraddr));
	////////////////////////////////////////////////
	struct ip_mreq mreq = { 0 };
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICASTIP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(sock, IPPROTO_IP,
		IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
	SOCKADDR_IN peeraddr;
	int addrlen = sizeof(peeraddr);
	char buf[BUFSIZE + 1];
	while (1) {
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			printf("recvfrom() error");
			break;
		}
		buf[retval] = '\0';
		printf("%s:%d - %s\n",
			inet_ntoa(peeraddr.sin_addr),
			ntohs(peeraddr.sin_port), buf);
	}
	////////////////////////////////////////////////
	retval = setsockopt(sock, IPPROTO_IP,
		IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
	closesocket(sock);
	WSACleanup();
	return 0;
}
