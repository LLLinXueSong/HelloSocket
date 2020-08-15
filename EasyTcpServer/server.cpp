
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
//			CELLLog::Info("exit cmdThread.....\n");
//			break;
//		}
//		else {
//			CELLLog::Info("cmd error....\n");
//		}
//	}
//
//
//}
class MyServer :public EasyTcpServer {
public:
	//���̲߳���ȫ
	virtual void OnLeave(CellClient* pClient) {
		_clientCount--;
		CELLLog::Info("client<%d> exit\n", pClient->sockfd());
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
			//CELLLog::Info("recv socket-%d cmd:netmsg_Login Len:%d username:%s password:%s\n", cSock, netmsg_Login->dataLength, netmsg_Login->userName, netmsg_Login->PassWord);
			netmsg_LoginR ret;
			if(0 == pClient->SendData(&ret)){
				//���ͻ��������ˣ�û����ȥ
				CELLLog::Info("<Socket = %d> send full\n", pClient->sockfd());
			}
			//netmsg_LoginR* ret = new netmsg_LoginR();
			//pCellServer->addSendTask(pClient,ret);  //�����ͽ��ԭ����Ҫ�ȴ�������ϲ��ܽ��������ݣ�����ֱ�Ӽӵ����Ͷ��У����̸߳�����
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
			//CELLLog::Info("recv socket-%d cmd:netmsg_Logout Len:%d username:%s\n", cSock, netmsg_Logout->dataLength, netmsg_Logout->userName);
			netmsg_LogoutR ret;
			//SendData(cSock, &ret);
			break;
		}
		default:
			CELLLog::Info("<socket=%d>, undefine msg, Len=%d\n", pClient->sockfd(), header->dataLength);
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
	CELLLog::Instance().setLogPath("serverLog.txt", "w");
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(2);
	//std::thread t1(cmdThread);
	//t1.detach();
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			server.Close();
			CELLLog::Info("exit cmdThread.....\n");
			break;
		}
		else {
			CELLLog::Info("cmd error....\n");
		}
	}
	CELLLog::Info("server exit...\n");
	//while (true) {
	//	Sleep(1);
	//}
	return 0;
}
