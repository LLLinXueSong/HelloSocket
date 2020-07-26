#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _WIN32
#ifdef _WIN32
	#include<Windows.h>
	#include<WinSock2.h>
#else
	#include<unistd.h>
	#include<arpa/inet.h>
	#include<string.h>
	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR           (-1)
#endif

#include<stdio.h>
#include<thread>

#pragma comment(lib,"ws2_32.lib")
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;
	short cmd;
};
struct Login :DataHeader
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};
struct LoginResult :DataHeader
{
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 1;
	}
	int result;
};
struct Logout :DataHeader
{
	Logout() {
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
	char PassWord[32];
};
struct LogoutResult :DataHeader
{
	LogoutResult() {
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 1;
	}
	int result;
};
struct NewUserJoin :DataHeader
{
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
int processor(SOCKET _cSock)
{
	char szRecv[1024] = {};
	int nLen = recv(_cSock, (char*)&szRecv, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("socket-%d client exit", _cSock);
		return -1;
	}
	//printf("cmd:%d Len:%d\n", header.cmd, header.dataLength);
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		LoginResult *login;
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		login = (LoginResult*)szRecv;
		printf("recv server cmd:loginresult Len:%d \n", login->dataLength);
		break;
	}
	case CMD_LOGOUT_RESULT:
	{
		LogoutResult *logout;
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		logout = (LogoutResult*)szRecv;
		printf("recv server cmd:loginresult Len:%d \n", logout->dataLength);
		break;
	}
	case CMD_NEW_USER_JOIN:
	{
		NewUserJoin *userJoin;
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		userJoin = (NewUserJoin*)szRecv;
		printf("new user join cmd:loginresult Len:%d \n", _cSock, userJoin->dataLength);
		break;
	}
	}
}
bool g_bRun = true;
void cmdThread(SOCKET _sock) {
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			g_bRun = false;
			printf("exit cmdThread.....\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login;
			strcpy(login.userName, "lyd");
			strcpy(login.PassWord, "lydmima");
			send(_sock, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy(logout.userName, "lyd");
			send(_sock, (const char*)&logout, sizeof(Logout), 0);
		}
		else {
			printf("cmd error....");
		}
	}


}
int main()
{
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET) {
		printf("create error\n");
	}
	else {
		printf("create success\n");
	}
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("192.168.0.103");
#else
	_sin.sin_addr.s_addr = inet_addr("192.168.0.105");
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
	if (ret == SOCKET_ERROR) {
		printf("connect error\n");
	}
	else {
		printf("connect success\n");
	}
	std::thread tl(cmdThread, _sock);
	tl.detach();
	while (g_bRun) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 1,0 };
		int ret = select(_sock, &fdReads, 0, 0, &t);
		if (ret < 0) {
			printf("select is over\n");
			break;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock)) {
				printf("select is over\n");
				break;
			}
		}
		//printf("we can do other\n");

	}
#ifdef _WIN32
	closesocket(_sock);
	WSACleanup();
#else
	close(_sock);
#endif
	printf("exit \n");

	getchar();
	return 0;
}