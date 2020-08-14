
#include<stdio.h>
#include<vector>
#include "EasyTcpServer.hpp"
#include<thread>


//bool g_bRun = 1;
//void cmdThread() {
//	while (true) {
//		char cmdBuf[256] = {};
//		scanf("%s", cmdBuf);
//		if (0 == strcmp(cmdBuf, "exit")) {
//			g_bRun = false;
//			printf("exit cmdThread.....\n");
//			break;
//		}
//		else {
//			printf("cmd error....\n");
//		}
//	}
//
//
//}
class MyServer :public EasyTcpServer {
public:
	//多线程不安全
	virtual void OnLeave(CellClient* pClient) {
		_clientCount--;
		printf("client<%d> exit\n", pClient->sockfd());
	}
	virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, netmsg_DataHeader* header) {
		EasyTcpServer::OnNetMsg(pCellServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTHeart();
			netmsg_Login *Login;
			Login = (netmsg_Login*)header;
			//printf("recv socket-%d cmd:netmsg_Login Len:%d username:%s password:%s\n", cSock, netmsg_Login->dataLength, netmsg_Login->userName, netmsg_Login->PassWord);
			netmsg_LoginR ret;
			if(0 == pClient->SendData(&ret)){
				//发送缓冲区满了，没发出去
			}
			pClient->SendData(&ret);
			//netmsg_LoginR* ret = new netmsg_LoginR();
			//pCellServer->addSendTask(pClient,ret);  //将发送解耦，原来需要等待发送完毕才能接收新数据，现在直接加到发送队列，新线程负责发送
			break;
		}
		case CMD_C2S_HEART: {
			pClient->resetDTHeart();
			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
			break;
		}
		
		case CMD_LOGOUT:
		{
			netmsg_Logout *Logout;
			Logout = (netmsg_Logout*)header;
			//printf("recv socket-%d cmd:netmsg_Logout Len:%d username:%s\n", cSock, netmsg_Logout->dataLength, netmsg_Logout->userName);
			netmsg_LogoutR ret;
			//SendData(cSock, &ret);
			break;
		}
		default:
			printf("<socket=%d>, undefine msg, Len=%d\n", pClient->sockfd(), header->dataLength);
			netmsg_DataHeader ret;
			//SendData(cSock, &ret);
			break;
		}
	}
	virtual void OnNetJoin(CellClient* pClient) {
		EasyTcpServer::OnNetJoin(pClient);		
	}
	virtual void OnNetLeave(CellClient* pClient) {
		EasyTcpServer::OnLeave(pClient);
	}

private:
};
int main()
{

	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(1);
	//std::thread t1(cmdThread);
	//t1.detach();
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			server.Close();
			printf("exit cmdThread.....\n");
			break;
		}
		else {
			printf("cmd error....\n");
		}
	}
	printf("server exit...\n");
	while (true) {
		Sleep(1);
	}
	return 0;
}
