
#include "EasyTcpClient.hpp"
#include<thread>
bool g_bRun = true;
//启动客户端数量
const int cCount = 10;
//发送线程数量
const int tCount = 4;
EasyTcpClient *client[cCount];
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
void sendThread(int id) {
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
		printf("Connect:%d\n", i);
	}
	std::chrono::milliseconds t(1000);
	std::this_thread::sleep_for(t);
	Login login[10];
	for (int i = 0; i < 10; i++) {
		strcpy(login[i].userName, "lyd");
		strcpy(login[i].PassWord, "lydmima");
	}
	int nLen = sizeof(login);
	while (g_bRun) {
		for (int i = begin; i < end; i++) {
			client[i]->SendData(login,nLen);
			client[i]->OnRun();
		}
	}
	for (int i = begin; i < end; i++) {
		client[i]->Close();
	}
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
	//for (int i = 0; i < cCount; i++) {
	//	if (!g_bRun) {
	//		return 0;
	//	}
	//	client[i] = new EasyTcpClient();
	//	client[i]->initSocket();
	//}
	//for (int i =0; i < cCount; i++) {
	//	if (!g_bRun) {
	//		return 0;
	//	}
	//	Sleep(100);
	//	client[i]->Connect("127.0.0.1", 4567);
	//	printf("Connect:%d\n", i);
	//}

	//Login login;
	//strcpy(login.userName, "lyd");
	//strcpy(login.PassWord, "lydmima");
	//while (g_bRun) {
	//	for (int i = 0; i < cCount; i++) {
	//		client[i]->SendData(&login);
	//		client[i]->OnRun();
	//	}
	//}
	//for (int i = 0; i < cCount; i++) {
	//	client[i]->Close();
	//}
	while (g_bRun)
		Sleep(100);
	getchar();
	return 0;
}