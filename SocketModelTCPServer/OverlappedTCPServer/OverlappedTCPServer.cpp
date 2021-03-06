// OverlappedTCPServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
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
HANDLE hReadEvent, hWriteEvent;
SOCKET client_sock;
void CALLBACK CompletionRoutine(
	DWORD dwError, DWORD cbTransferred, 
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags) {
	SOCKETINFO *ptr = (SOCKETINFO*)lpOverlapped;
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
			&recvbytes, &flags, &ptr->overlapped, CompletionRoutine);
	}
}
DWORD WINAPI WorkerThread(LPVOID arg) {
	while (1) {
		while (1) {
			DWORD result =
				WaitForSingleObjectEx(
					hWriteEvent, INFINITE, TRUE
				);
			if (result == WAIT_OBJECT_0) break;
			if (result != WAIT_IO_COMPLETION)
				return 1;
		}
		SOCKETINFO *ptr = new SOCKETINFO;
		if (ptr == NULL) return 1;
		ZeroMemory(&ptr->overlapped,sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;
		SetEvent(hReadEvent);

		DWORD recvbytes, flags = 0;
		int retval = WSARecv(ptr->sock, &ptr->wsabuf, 1,
			&recvbytes, &flags, &ptr->overlapped,
			CompletionRoutine);
	}
	return 0;
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
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	HANDLE hThread = CreateThread(NULL, 0, WorkerThread, NULL,
		0, NULL);
	CloseHandle(hThread);
	while (1) {
		WaitForSingleObject(hReadEvent, INFINITE);
		client_sock = accept(listen_sock, NULL, NULL);
		SetEvent(hWriteEvent);
	}
	///////////////////////////////////////////////////
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}

