#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
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
		dataLength = sizeof(LogoutResult);
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
			printf("recv server cmd:loginresult Len:%d \n", _cSock, login->dataLength);
			break;
		}
		case CMD_LOGOUT:
		{
			LogoutResult *logout;
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			logout = (LogoutResult*)szRecv;
			printf("recv server cmd:loginresult Len:%d \n", _cSock, logout->dataLength);
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
int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
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
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
	if (ret == SOCKET_ERROR) {
		printf("connect error\n");
	}
	else {
		printf("connect success\n");
	}
	while (true) {
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
		printf("we can do other\n");
		Login login;
		strcpy(login.userName, "lyd");
		strcpy(login.PassWord, "lydmima");
		send(_sock, (const char*)&login, sizeof(Login), 0);
	}

	closesocket(_sock);
	WSACleanup();
	printf("exit \n");

	getchar();
	return 0;
}