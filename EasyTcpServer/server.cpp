

#include "EasyTcpServer.hpp"
#include "CELLMsgStream.hpp"

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
	//多线程不安全
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
				//发送缓冲区满了，没发出去
				CELLLog::Info("<Socket = %d> send full\n", pClient->sockfd());
			}
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
			CELLRecvStream r(header);
			auto n1 = r.ReadInt8();
			auto n2 = r.ReadInt16();
			auto n3 = r.ReadInt32();
			auto n4 = r.ReadFloat();
			auto n5 = r.ReadDouble();
			char username[32] = {};
			auto n8 = r.ReadArray(username, 32);
			char pw[32] = {};
			auto n6 = r.ReadArray(pw, 32);
			int ata[10] = {};
			auto n7 = r.ReadArray(ata, 10);
			

			CELLSendStream s;
			s.setNetCmd(CMD_LOGOUT_RESULT);
			s.WriteInt8(5);
			s.WriteInt16(5);
			s.WriteInt32(5);
			s.WriteFloat(5.0f);
			s.WriteDouble(5.0);
			char* str = "Server";
			s.WriteArray(str, strlen(str));
			char a[] = "ahaa";
			s.WriteArray(a, strlen(a));
			int b[] = { 1,2,3,4,5 };
			s.WriteArray(b, 5);
			s.finish();
			pClient->SendData(s.data(), s.length());
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
	server.Start(1);
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
