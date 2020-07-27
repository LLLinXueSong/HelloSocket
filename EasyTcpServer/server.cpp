
#include<stdio.h>
#include<vector>
#include "EasyTcpServer.hpp"
#include<thread>
bool g_bRun = 1;
void cmdThread() {
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			g_bRun = false;
			printf("exit cmdThread.....\n");
			break;
		}
		else {
			printf("cmd error....\n");
		}
	}


}
int main()
{

	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4566);
	server.Listen(10);
	std::thread t1(cmdThread);
	t1.detach();
	while (g_bRun) {
		server.OnRun();
	}
	server.Close();
	printf("server exit...\n");
	getchar();
	return 0;
}
