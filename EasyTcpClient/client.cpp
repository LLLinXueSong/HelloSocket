
#include "EasyTcpClient.hpp"
#include<thread>

void cmdThread(EasyTcpClient* client) {
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			client->Close();
			printf("exit cmdThread.....\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login;
			strcpy(login.userName, "lyd");
			strcpy(login.PassWord, "lydmima");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy(logout.userName, "lyd");
			client->SendData(&logout);
		}
		else {
			printf("cmd error....\n");
		}
	}


}
int main()
{
	EasyTcpClient client;
	client.initSocket();
	client.Connect("127.0.0.1",4566);
	std::thread t1(cmdThread, &client);
	t1.detach();
	while (client.isRun()) {
		client.OnRun();
	}
	getchar();
	return 0;
}