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

#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 10240
#endif // !RECV_BUFF_SIZE
#include<stdio.h>
#include<vector>
#include "MessageHeader.hpp"
class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}
	SOCKET sockfd() {
		return _sockfd;
	}
	char* msgBuf() {
		return _szMsgBuf;
	}
	int getLast() {
		return _lastPos;
	}
	void setLastPos(int pos) {
		_lastPos = pos;
	}
private:
	SOCKET _sockfd;
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	int _lastPos = 0;
};
class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
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
		SOCKET cSock = INVALID_SOCKET;
		char msgBuf[] = "hello,I'm Server\n";
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif // _WIN32

		if (INVALID_SOCKET == cSock) {
			printf("socket = %d 接收无效客户端\n",(int)_sock);
		}
		else {
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			printf("socket=<%d> 新客户端加入socket = %d IP:%s \n", (int)_sock,(int)cSock, inet_ntoa(clientAddr.sin_addr));
			_clients.push_back(new ClientSocket(cSock));
		}
		return cSock;
	}
	//关闭socket
	void Close() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = 0; n < _clients.size(); n++)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			printf("exit \n");
			closesocket(_sock);
			WSACleanup();
#else
			for (int n = 0; n < _clients.size(); n++)
			{
				close(_clients[n]);
				delete _clients[n];
			}
			printf("exit \n");
			close(_sock);
#endif
			_clients.clear();
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
			for (int n = 0; n < _clients.size(); n++)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock<_clients[n]->sockfd()) {
					maxSock = _clients[n]->sockfd();
				}
			}
			//文件描述符最大值+1，windows中可以写0
			timeval t = { 0,0 };
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
			for (int n = 0; n < _clients.size(); n++)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead)) {
					if (-1 == RecvData(_clients[n])) {
						std::vector<ClientSocket*>::iterator iter = _clients.begin() + n;
						if (iter != _clients.end()) {
							_clients.erase(iter);
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

	char _szRecv[RECV_BUFF_SIZE] = {};
	//接受数据 处理粘包 拆分包
	int RecvData(ClientSocket* pClient)
	{
		
		int nLen = (int)recv(pClient->sockfd(), _szRecv, sizeof(DataHeader), 0);
		if (nLen <= 0) {
			printf("socket-%d client exit\n", pClient->sockfd());
			return -1;
		}
		//将收取到数据拷贝到消息缓冲区
		memcpy(pClient->msgBuf()+pClient->getLast(), _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLast() + nLen);
		while (pClient->getLast() >= sizeof(DataHeader)) {
			DataHeader *header = (DataHeader*)pClient->msgBuf();
			if (pClient->getLast() >= header->dataLength) {
				//剩余未处理消息缓冲区的长度
				int nSize = pClient->getLast() - header->dataLength;
				OnNetMsg(pClient->sockfd(),header);
				//将未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf()+ header->dataLength, nSize);
				pClient->setLastPos(nSize);
			}
			else {
				break;
			}

		}

		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(SOCKET cSock,DataHeader *header) {
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login;
			login = (Login*)header;
			//printf("recv socket-%d cmd:login Len:%d username:%s password:%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			LoginResult ret;
			SendData(cSock, &ret);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout *logout;
			logout = (Logout*)header;
			printf("recv socket-%d cmd:logout Len:%d username:%s\n", cSock, logout->dataLength, logout->userName);
			LogoutResult ret;
			SendData(cSock, &ret);
			break;
		}
		default:
			printf("<socket=%d>, undefine msg, Len=%d\n", cSock, header->dataLength);
			DataHeader ret;
			SendData(cSock, &ret);
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
		for (int n = 0; n < _clients.size(); n++)
		{
			SendData(_clients[n]->sockfd(), header);
		}
		
	}
};


#endif