
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
class MyServer:public EasyTcpServer {
public:
	//多线程不安全
	virtual void OnLeave(ClientSocket* pClient) {
		_clientCount--;
		printf("client<%d> exit\n", pClient->sockfd());
	}
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header) {
		_recvCount++;
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login;
			login = (Login*)header;
			//printf("recv socket-%d cmd:login Len:%d username:%s password:%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			LoginResult ret;
			pClient->SendData(&ret);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout *logout;
			logout = (Logout*)header;
			//printf("recv socket-%d cmd:logout Len:%d username:%s\n", cSock, logout->dataLength, logout->userName);
			LogoutResult ret;
			//SendData(cSock, &ret);
			break;
		}
		default:
			printf("<socket=%d>, undefine msg, Len=%d\n", pClient->sockfd(), header->dataLength);
			DataHeader ret;
			//SendData(cSock, &ret);
			break;
		}
	}
	virtual void OnNetJoin(ClientSocket* pClient) {
		_clientCount++;
		printf("client<%d> join\n", pClient->sockfd());
	}
private:
};
int main()
{

	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(4);
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
