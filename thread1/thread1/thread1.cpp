// thread1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
struct Point3D { int x, y, z; };
DWORD WINAPI MyThread(LPVOID arg) {
	Point3D *pt = (Point3D *)arg;
	while (1) {
		printf("%d:%d,%d,%d\n",
			GetCurrentThreadId(), pt->x, pt->y, pt->z);
		Sleep(1000);
	}
	return 0;
}
int main()
{
	Point3D pt1 = { 10,20,30 }, pt2 = { 40,50,60 };
	HANDLE hThread1 = CreateThread(NULL, 0, MyThread,
		&pt1, 0, NULL);
	HANDLE hThread2 = CreateThread(NULL, 0, MyThread,
		&pt2, 0, NULL);
	while (1) {
		printf("main:%d\n", GetCurrentThreadId());
		Sleep(500);
	}
    return 0;
}

