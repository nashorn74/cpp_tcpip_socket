// CompletionPortTCPServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#define SERVERPORT 9000
#define BUFSIZE 512
struct SOCKETINFO {
	WSAOVERLAPPED overlapped;
	SOCKET sock; char buf[BUFSIZE + 1];
	int recvbytes; int sendbytes;
	WSABUF wsabuf;
};
DWORD WINAPI WorkerThread(LPVOID arg) {
	HANDLE hcp = (HANDLE)arg;
	while (1) {
		DWORD cbTransferred;
		SOCKET client_sock;
		SOCKETINFO *ptr;
		int retval = GetQueuedCompletionStatus(hcp,
			&cbTransferred, (PULONG_PTR)&client_sock,
			(LPOVERLAPPED*)&ptr, INFINITE);
		if (retval == 0 || cbTransferred == 0) {
			closesocket(ptr->sock);
			delete ptr;
			continue;
		}
		if (ptr->recvbytes == 0) {
			ptr->recvbytes = cbTransferred;
			ptr->sendbytes = 0;
			ptr->buf[ptr->recvbytes] = '\0';
			printf("%s\n", ptr->buf);
		}
		{
			ptr->recvbytes = 0;
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUFSIZE;
			DWORD recvbytes, flags = 0;
			int retval = WSARecv(ptr->sock, &ptr->wsabuf, 1,
				&recvbytes, &flags, &ptr->overlapped, NULL);
		}
	}
	return 0;
}
int main()
{
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
	
	HANDLE hcp = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE, NULL, 0, 0);
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	HANDLE hThread;
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++) {
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		CloseHandle(hThread);
	}
	
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN serveraddr = { 0 };
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = bind(listen_sock, (SOCKADDR*)&serveraddr,
		sizeof(serveraddr));
	retval = listen(listen_sock, SOMAXCONN);
	///////////////////////////////////////////////////
	while (1) {
		SOCKADDR_IN clientaddr = { 0 };
		int addrlen = sizeof(clientaddr);
		SOCKET client_sock = accept(listen_sock,
			(SOCKADDR*)&clientaddr, &addrlen);
		CreateIoCompletionPort((HANDLE)client_sock,
			hcp, client_sock, 0);
		SOCKETINFO *ptr = new SOCKETINFO;
		if (ptr == NULL) return 1;
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		DWORD recvbytes, flags = 0;
		int retval = WSARecv(ptr->sock, &ptr->wsabuf, 1,
			&recvbytes, &flags, &ptr->overlapped, NULL);
	}
	///////////////////////////////////////////////////
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
