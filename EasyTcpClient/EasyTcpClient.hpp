#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
	#include<Windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")

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
#include "MessageHeader.hpp"
class EasyTcpClient
{
public:
	EasyTcpClient() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpClient() {
		Close();
	}
	//初始化socket
	void initSocket() {
		//启动win socket 2.x环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//避免重复创建 先关闭
		if (_sock != INVALID_SOCKET) { 
			printf("socket = %d  关闭旧连接。。。。\n",_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock == INVALID_SOCKET) {
			printf("create socket error\n");
		}
		else {
			printf("create socket success\n");
		}
	}
	//连接服务器
	int Connect(char* ip, short port) {
		if (_sock == INVALID_SOCKET) {
			initSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
		if (ret == SOCKET_ERROR) {
			printf("connect error\n");
		}
		else {
			printf("connect success\n");
		}
		return ret;
	}
	//关闭socket
	void Close() {
		if (_sock != INVALID_SOCKET) { //避免重复关闭
#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
			printf("client exit \n");
			_sock = INVALID_SOCKET;
		}

	}
	//发送数据
	//接受数据
	//处理网络消息
	bool OnRun() {
		if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 1,0 };
			int ret = select(_sock, &fdReads, 0, 0, &t);
			if (ret < 0) {
				printf("socket = %d  select is over\n", _sock);
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock)) {
					printf("socket = %d  select is over\n", _sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}

	//接受数据 处理粘包 拆分包
	int RecvData(SOCKET _cSock)
	{
		char szRecv[1024] = {};
		int nLen = recv(_cSock, (char*)&szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("socket-%d client exit", _cSock);
			return -1;
		}
		//printf("cmd:%d Len:%d\n", header.cmd, header.dataLength);
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		return 0;
	}
	//响应网络消息
	void OnNetMsg(DataHeader *header) {
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *login;
			
			login = (LoginResult*)header;
			printf("recv server cmd:loginresult Len:%d \n", login->dataLength);
			break;
		}
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult *logout;
			
			logout = (LogoutResult*)header;
			printf("recv server cmd:loginresult Len:%d \n", logout->dataLength);
			break;
		}
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin *userJoin;
			
			userJoin = (NewUserJoin*)header;
			printf("new user join cmd:loginresult Len:%d \n",  userJoin->dataLength);
			break;
		}
		}
	}
	//发送数据
	int SendData(DataHeader *header) {
		if (isRun() && header) {
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:
	SOCKET _sock;
};





#endif