
#include "EasyTcpClient.hpp"
#include<thread>
bool g_bRun = true;
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
	const int cCount = 10;
	EasyTcpClient *client[cCount];
	for (int i = 0; i < cCount; i++) {
		client[i] = new EasyTcpClient();
		client[i]->initSocket();
		client[i]->Connect("127.0.0.1", 4566);
	}
	
	std::thread t1(cmdThread);
	t1.detach();
	Login login;
	strcpy(login.userName,"lyd");
	strcpy(login.PassWord, "lydmima");
	while (g_bRun) {
		for (int i = 0; i < cCount; i++) {
			client[i]->SendData(&login);
			client[i]->OnRun();
		}
	}
	for (int i = 0; i < cCount; i++) {
		client[i]->Close();
	}
	
	getchar();
	return 0;
}