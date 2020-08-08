#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#include<iostream>
//#pragma comment(lib,"ws2_32.lib")
int main()
{
	int a = 13452;
	double b = 1.000342;
	printf("%d\n", int(a / b));
	getchar();
	return 0;
}