
#include<stdio.h>
#include<vector>
#include "EasyTcpServer.hpp"



int main()
{
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4566);
	server.Listen(5);
	while (server.isRun()) {
		server.OnRun();
	}
	server.Close();
	printf("server exit...\n");
	getchar();
	return 0;
}
