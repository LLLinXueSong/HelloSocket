#include "CELLTimestamp.hpp"
#include "EasyTcpClient.hpp"
#include<thread>
#include<atomic>
bool g_bRun = true;
//启动客户端数量
const int cCount = 4;
//发送线程数量
const int tCount = 2;
std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;
EasyTcpClient *client[cCount];

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
			printf("exit cmdThread.....\n"); 
			break;
		}
		else {
			printf("cmd error....\n");
		}
	}
}
void sendThread(int id) {
	printf("thread<%d>  start\n",id);
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id*c;
	for (int i = begin; i < end; i++) {
		if (!g_bRun) {
			return ;
		}
		client[i] = new EasyTcpClient();
		client[i]->initSocket();
	}
	for (int i = begin; i < end; i++) {
		if (!g_bRun) {
			return ;
		}
		Sleep(100);
		client[i]->Connect("127.0.0.1", 4567);
		
	}
	printf("Connect:begin=%d, end=%d\n", begin,end);
	readyCount++;
	while (readyCount < tCount) {
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}

	std::thread t1(recvThread, begin, end);
	t1.detach();
	netmsg_Login login[1];
	for (int i = 0; i < 1; i++) {
		strcpy_s(login[i].userName, "lyd");
		strcpy_s(login[i].PassWord, "lydmima");
	}
	int nLen = sizeof(login);
	while (g_bRun) {
		for (int i = begin; i < end; i++) {
			if (SOCKET_ERROR != client[i]->SendData(login, nLen)) {
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
	printf("thread<%d> exit\n", id);
}

int main()
{
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
			printf("thread<%d>,clients<%d>,time<%lf>,send<%d>\n", tCount, cCount,t,sendCount);
			sendCount = 0;
			tTime.update();
		}
		Sleep(1);
	}
		
	getchar();
	return 0;
}