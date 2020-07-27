#ifndef _EasyTcpServer_hpp_
#define  _EasyTcpServer_hpp_
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
#endif // _WIN32

#include<stdio.h>
#include<vector>
#include "MessageHeader.hpp"
class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer() {
		Close();
	}
	//初始化socket
	SOCKET InitSocket() {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32
		//避免重复创建 先关闭
		if (_sock != INVALID_SOCKET) {
			printf("socket = %d  关闭旧连接。。。。\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock == INVALID_SOCKET) {
			printf("create socket error\n");
		}
		else {
			printf("create socket = %d  success\n", (int)_sock);
		}
		return _sock;
	}
	//绑定端口号
	int Bind(const char* ip,unsigned short port) {
		/*if (INVALID_SOCKET == _sock) {
			InitSocket();
		}*/
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
	}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
		_sin.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (ret == SOCKET_ERROR) {
			printf("绑定端口 <%d> ERROR\n",port);
		}
		else {

			printf("绑定端口 <%d> 成功\n",port);
		}
		return ret;
	}
	//监听端口号
	int Listen(int n) {
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("socket = %d监听ERROR\n",_sock);
		}
		else {

			printf("socket = %d监听成功\n",_sock);
		}
		return ret;
	}
	//接收客户连接
	SOCKET Accept() {
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
		char msgBuf[] = "hello,I'm Server\n";
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif // _WIN32

		if (INVALID_SOCKET == _cSock) {
			printf("socket = %d 接收无效客户端\n",(int)_sock);
		}
		else {
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			printf("socket=<%d> 新客户端加入socket = %d IP:%s \n", (int)_sock,(int)_cSock, inet_ntoa(clientAddr.sin_addr));
			g_clients.push_back(_cSock);
		}
		return _cSock;
	}
	//关闭socket
	void Close() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = 0; n < g_clients.size(); n++)
			{
				closesocket(g_clients[n]);
			}
			printf("exit \n");
			closesocket(_sock);
			WSACleanup();
#else
			for (int n = 0; n < g_clients.size(); n++)
			{
				close(g_clients[n]);
			}
			printf("exit \n");
			close(_sock);
#endif
		}
	}

	//处理网络消息
	bool OnRun() {
		if (isRun()) {
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;
			for (int n = 0; n < g_clients.size(); n++)
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock<g_clients[n]) {
					maxSock = g_clients[n];
				}
			}
			//文件描述符最大值+1，windows中可以写0
			timeval t = { 1,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0) {
				printf("select is over\n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = 0; n < g_clients.size(); n++)
			{
				if (FD_ISSET(g_clients[n], &fdRead)) {
					if (-1 == RecvData(g_clients[n])) {
						std::vector<SOCKET>::iterator iter = g_clients.begin() + n;
						if (iter != g_clients.end()) {
							g_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}

	//接受数据 处理粘包 拆分包
	int RecvData(SOCKET _cSock)
	{
		char szRecv[1024] = {};
		int nLen = (int)recv(_cSock, (char*)&szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("socket-%d client exit\n", _cSock);
			return -1;
		}
		//printf("cmd:%d Len:%d\n", header.cmd, header.dataLength);
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(SOCKET _cSock,DataHeader *header) {
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login;
			
			login = (Login*)header;
			printf("recv socket-%d cmd:login Len:%d username:%s password:%s\n", _cSock, login->dataLength, login->userName, login->PassWord);
			LoginResult ret;
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout *logout;
			logout = (Logout*)header;
			printf("recv socket-%d cmd:logout Len:%d username:%s\n", _cSock, logout->dataLength, logout->userName);
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
	//发送指定客户数据
	int SendData(SOCKET _cSock,DataHeader *header) {
		if (isRun() && header) {
			return send(_cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//群发数据
	void SendDataToAll(DataHeader *header) {
		for (int n = 0; n < g_clients.size(); n++)
		{
			SendData(g_clients[n], header);
		}
		
	}
};


#endif