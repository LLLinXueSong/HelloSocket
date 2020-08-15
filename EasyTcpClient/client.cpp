#include "CELLTimestamp.hpp"
#include "EasyTcpClient.hpp"
#include<thread>
#include<atomic>
bool g_bRun = true;
//启动客户端数量
const int cCount = 1;
//发送线程数量
const int tCount = 1;
std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;
EasyTcpClient *client[cCount];

class MyClient:public EasyTcpClient {
public:
	virtual void OnNetMsg(netmsg_DataHeader *header) {
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			netmsg_LoginR *login;
			login = (netmsg_LoginR*)header;
			//CELLLog::Info("<socket=%d>recv server cmd:netmsg_LoginR Len:%d \n",_sock, login->dataLength);
			break;
		}
		case CMD_LOGOUT_RESULT:
		{
			netmsg_LogoutR *logout;
			logout = (netmsg_LogoutR*)header;
			CELLLog::Info("<socket=%d> recv server cmd:netmsg_LoginR Len:%d \n", pClient->sockfd(), logout->dataLength);
			break;
		}
		case CMD_NEW_USER_JOIN:
		{
			netmsg_NewUserJoin *userJoin;
			userJoin = (netmsg_NewUserJoin*)header;
			CELLLog::Info("<socket=%d> new user join cmd:netmsg_LoginR Len:%d \n", pClient->sockfd(), userJoin->dataLength);
			break;
		}
		case CMD_ERROR:
		{
			CELLLog::Info("<socket=%d> cmd:CMD_ERROR Len:%d \n", header->dataLength);
			break;
		}
		default: {
			CELLLog::Info("<socket=%d> undefine msg \n", pClient->sockfd());
		}
		}
	}
private:
};
void recvThread(int begin, int end) {
	while (g_bRun) {
		for (int i = begin; i < end; i++) {
			client[i]->OnRun();
		}
	}
}
void cmdThread() {
	while (true) {
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			g_bRun = false;
			CELLLog::Info("exit cmdThread.....\n"); 
			break;
		}
		else {
			CELLLog::Info("cmd error....\n");
		}
	}
}
void sendThread(int id) {
	CELLLog::Info("thread<%d>  start\n",id);
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id*c;
	for (int i = begin; i < end; i++) {
		if (!g_bRun) {
			return ;
		}
		client[i] = new MyClient();
		client[i]->initSocket();
	}
	for (int i = begin; i < end; i++) {
		if (!g_bRun) {
			return ;
		}
		Sleep(10);
		client[i]->Connect("127.0.0.1", 4567);
		
	}
	CELLLog::Info("Connect:begin=%d, end=%d\n", begin,end);
	readyCount++;
	while (readyCount < tCount) {
		std::chrono::milliseconds t(1);
		std::this_thread::sleep_for(t);
	}
	netmsg_Login login[1];
	for (int i = 0; i < 1; i++) {
		strcpy_s(login[i].userName, "lyd");
		strcpy_s(login[i].PassWord, "lydmima");
	}
	int nLen = sizeof(login);
	std::thread t1(recvThread, begin, end);
	t1.detach();
	while (g_bRun) {
		for (int i = begin; i < end; i++) {
			if (SOCKET_ERROR != client[i]->SendData(login)) {
				sendCount++;
			}
		}
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}
	for (int i = begin; i < end; i++) {
		client[i]->Close();
		delete client[i];
	}
	CELLLog::Info("thread<%d> exit\n", id);
}

int main()
{
	CELLLog::Instance().setLogPath("clientLog.txt", "w");
	//启动发送线程
	for (int n = 0; n < tCount; n++) {
		std::thread t1(sendThread, n + 1);
		t1.detach();
	}
	std::thread t1(cmdThread);
	t1.detach();
	CELLTimestamp tTime;
	while (g_bRun) {
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0) {
			CELLLog::Info("thread<%d>,clients<%d>,time<%lf>,send<%d>\n", tCount, cCount,t,sendCount.load());
			sendCount = 0;
			tTime.update();
		}
		Sleep(1);
	}
		
	getchar();
	return 0;
}