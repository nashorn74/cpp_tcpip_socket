// event.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#define BUFSIZE 10
HANDLE hReadEvent;
HANDLE hWriteEvent;
int buf[BUFSIZE];//공유자원
DWORD WINAPI WriteThread(LPVOID arg) {
	for (int k = 1; k <= 500; k++) {
		WaitForSingleObject(hReadEvent, INFINITE);
		for (int i = 0; i < BUFSIZE; i++)
			buf[i] = k;
		SetEvent(hWriteEvent);
	}
	return 0;
}
DWORD WINAPI ReadThread(LPVOID arg) {
	while (1) {
		WaitForSingleObject(hWriteEvent, INFINITE);
		printf("%4d:", GetCurrentThreadId());
		for (int i = 0; i < BUFSIZE; i++)
			printf("%3d ", buf[i]);
		printf("\n");
		ZeroMemory(buf, sizeof(buf));
		SetEvent(hReadEvent);
	}
	return 0;
}
int main()
{
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	HANDLE hThread[4];
	hThread[0] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	hThread[2] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	hThread[3] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	WaitForMultipleObjects(4, hThread, TRUE, INFINITE);
	CloseHandle(hWriteEvent); CloseHandle(hReadEvent);
	for (int i = 0; i < 3; i++) CloseHandle(hThread[i]);
	Sleep(5000);
    return 0;
}

