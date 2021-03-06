// thread2.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
int sum = 0;
DWORD WINAPI MyThread(LPVOID arg) {
	int num = (int)arg;
	for (int i = 1; i <= num; i++) sum += i;
	return 0;
}
int main()
{
	int num = 100000;
	HANDLE hThread = CreateThread(NULL, 0,
		MyThread, (LPVOID)num, CREATE_SUSPENDED, NULL);
	printf("sum=%d\n", sum);
	ResumeThread(hThread);
	WaitForSingleObject(hThread, INFINITE);
	printf("sum=%d\n", sum);
	CloseHandle(hThread);
    return 0;
}

