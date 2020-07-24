#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<vector>
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
std::vector<SOCKET> g_clients;
int processor(SOCKET _cSock)
{
	char szRecv[1024] = {};
	int nLen = recv(_cSock, (char*)&szRecv, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("socket-%d client exit\n",_cSock);
		return -1;
	}
	//printf("cmd:%d Len:%d\n", header.cmd, header.dataLength);
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		Login *login;
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		login = (Login*)szRecv;
		printf("recv socket-%d cmd:login Len:%d username:%s password:%s\n", _cSock,login->dataLength, login->userName, login->PassWord);
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
		break;
	}
	case CMD_LOGOUT:
	{
		Logout *logout;
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		logout = (Logout*)szRecv;
		printf("recv socket-%d cmd:logout Len:%d username:%s\n", _cSock,logout->dataLength, logout->userName);
		LogoutResult ret;
		send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
		break;
	}
	default:
		DataHeader header = { 0, CMD_ERROR };
		send(_cSock, (char*)&header, sizeof(header), 0);
		break;
	}
}
int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	SOCKET _sock = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) == SOCKET_ERROR){
		printf("绑定ERROR\n");
	}
	else{

		printf("绑定成功\n");
	}
	if (SOCKET_ERROR == listen(_sock, 5)){
		printf("监听ERROR\n");
	}
	else{

		printf("监听成功\n");
	}
	
	
	while (true){
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		for (int n = 0; n < g_clients.size(); n++)
		{
			FD_SET(g_clients[n], &fdRead);
		}
		//文件描述符最大值+1，windows中可以写0
		timeval t = { 1,0 };
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0) {
			printf("select is over\n");
			break;
		}
		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			char msgBuf[] = "hello,I'm Server\n";
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock) {
				printf("接收无效客户端\n");
			}
			else {
				for (int n = 0; n < g_clients.size(); n++)
					{
						NewUserJoin userJoin;
						send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
					}
					printf("新客户端加入socket = %d IP:%s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
					g_clients.push_back(_cSock);
			}
			
		}
		for (int n = 0; n< fdRead.fd_count; n++)
		{
			if (-1 == processor(fdRead.fd_array[n])){
				auto iter = find(g_clients.begin(),g_clients.end(),fdRead.fd_array[n]);
				if (iter != g_clients.end()) {
					g_clients.erase(iter);
				}
			}
		}
		//printf("we can do other\n");
			
	}
	for (int n = 0; n < g_clients.size(); n++)
	{
		closesocket(g_clients[n]);
	}
	printf("exit \n");
	WSACleanup();
	getchar();
	return 0;
}