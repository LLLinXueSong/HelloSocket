#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	char msgBuf[] = "hello,I'm Server\n";
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock) {
		printf("接收无效客户端\n");
	}
	printf("新客户端加入socket = %d IP:%s \n", (int)_cSock,inet_ntoa(clientAddr.sin_addr));
	char cmdBuf[128] = {};
	while (true){
		DataHeader header = {};
		int nLen = recv(_cSock,(char*)&header , sizeof(header), 0);
		if (nLen <= 0) {
			printf("client exit");
			break;
		}
		//printf("cmd:%d Len:%d\n", header.cmd, header.dataLength);
		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			Login login = {};
			recv(_cSock, (char*)&login+sizeof(DataHeader), sizeof(login)- sizeof(DataHeader), 0);
			printf("cmd:login Len:%d username:%s password:%s\n",  login.dataLength,login.userName,login.PassWord);
			LoginResult ret;
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout logout = {};
			recv(_cSock, (char*)&logout + sizeof(DataHeader), sizeof(Logout)- sizeof(DataHeader), 0);
			printf("cmd:logout Len:%d username:%s\n", logout.dataLength, logout.userName);
			LogoutResult ret;
			send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
			break;
		}
		default:
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_cSock, (char*)&header, sizeof(header), 0);
			break;
		}
		
	}
	
	closesocket(_sock);
	printf("exit \n");
	
	WSACleanup();
	getchar();
	return 0;
}