// AsyncSelectTCPServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
//#include <Windows.h>
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#define SERVERPORT 9000
#define BUFSIZE 512
#define WM_SOCKET (WM_USER+1)
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
void RemoveSocketInfo(SOCKET sock) {
	for (int i = 0; i < nTotalSockets; i++) {
		SOCKETINFO *ptr = SocketInfoArray[i];
		if (ptr->sock == sock) {
			closesocket(ptr->sock);
			delete ptr;	ptr = NULL;
			if (i != (nTotalSockets - 1))
				SocketInfoArray[i] =
				SocketInfoArray[nTotalSockets - 1];
			--nTotalSockets;
			break;
		}
	}
}
SOCKETINFO *GetSocketInfo(SOCKET sock) {
	for (int i = 0; i < nTotalSockets; i++) {
		SOCKETINFO *ptr = SocketInfoArray[i];
		if (ptr->sock == sock) return ptr;
	}
	return NULL;
}
void ProcessSocketMessage(HWND hWnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam) {
	if (WSAGETSELECTERROR(lParam)) {
		RemoveSocketInfo(wParam); return;
	}
	switch (WSAGETSELECTEVENT(lParam)) {
	case FD_ACCEPT:
		{
			SOCKADDR_IN clientaddr = { 0 };
			int addrlen = sizeof(clientaddr);
			SOCKET client_sock = accept(wParam,
				(SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock != INVALID_SOCKET) {
				AddSocketInfo(client_sock);
				int retval = WSAAsyncSelect(client_sock, hWnd,
					WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
				if (retval == SOCKET_ERROR) {
					RemoveSocketInfo(client_sock);
				}
			}
		}
		break;
	case FD_READ:
		{
			SOCKETINFO *ptr = GetSocketInfo(wParam);
			int retval = recv(ptr->sock, ptr->buf,
				BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				RemoveSocketInfo(wParam); return;
			} 
			ptr->buf[retval] = '\0';
			ptr->recvbytes = retval;
			printf("%s\n", ptr->buf);
		}
	case FD_WRITE:
		{
			SOCKETINFO *ptr = GetSocketInfo(wParam);
			if (ptr->recvbytes <= ptr->sendbytes) return;
			int retval = send(ptr->sock, ptr->buf + ptr->sendbytes,
				ptr->recvbytes - ptr->sendbytes, 0);
			if (retval == SOCKET_ERROR) {
				RemoveSocketInfo(wParam); return;
			}
			ptr->sendbytes += retval;
			if (ptr->recvbytes == ptr->sendbytes) {
				ptr->recvbytes = ptr->sendbytes = 0;
			}
		}
		break;
	case FD_CLOSE:
		RemoveSocketInfo(wParam);
		break;
	}
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_SOCKET://윈도OS가 소켓감지 결과를 전달
		ProcessSocketMessage(hWnd, uMsg, wParam, lParam);
		//MessageBox(hWnd, "WM_SOCKET", "알림", MB_OK);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
int main()
{
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0; wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = 
		(HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "MyWndClass";
	if (!RegisterClass(&wndclass)) return 1;
	HWND hWnd = CreateWindow("MyWndClass",
		"AsyncSelectTCPServer", WS_OVERLAPPEDWINDOW,
		0, 0, 600, 200, NULL, NULL, NULL, NULL);
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
    
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
	retval = WSAAsyncSelect(listen_sock, hWnd,
		WM_SOCKET, FD_ACCEPT | FD_CLOSE);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	closesocket(listen_sock);
	WSACleanup();
	return msg.wParam;
	//return 0;
}

