#ifndef _EasyTcpServer_hpp_
#define  _EasyTcpServer_hpp_
#ifdef _WIN32
	#define FD_SETSIZE 1024
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
#include<map>
#include<atomic>
#include<thread>
#include<stdio.h>
#include<vector>
#include<mutex>
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
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
	//发送指定客户数据
	int SendData(DataHeader *header) {
		if (header) {
			return send(_sockfd, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:
	SOCKET _sockfd;
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	int _lastPos = 0;

};
class INetEvent
{
public:
	//客户端加入
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	//有客户端离开时通知
	virtual void OnLeave(ClientSocket* pClient) = 0;
	//客户端消息事件
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header) = 0;
};
class CellServer {
private:
	//网络事件对象
	INetEvent* _pNetEvent;
	std::vector<ClientSocket*> _clientsBuff;
	//缓冲队列锁
	std::mutex _mutex;
	SOCKET _sock;
	std::thread _thread;
	std::map<SOCKET,ClientSocket*> _clients;
public:
	CellServer(SOCKET sock = INVALID_SOCKET) {
		_sock = sock;
		_pNetEvent = nullptr;
	}
	CellServer() {
		Close();
		_sock = INVALID_SOCKET;
	}
	void addClient(ClientSocket* pClient) {
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}
	void setEventObj(INetEvent* event) {
		_pNetEvent = event;
	}
	size_t getClientCount() {
		return _clients.size() + _clientsBuff.size();
	}
	void Start() {
		//thread 使用 void (*)(this) mem_fn可以将void(CellServer::*)(this) 转换
		//把成员函数转化为函数对象，使用对象指针或对象引用进行绑定
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
		
	}
	//备份socket fd_set
	fd_set _fdRead_bak;
	//客户是否发生改变
	bool _clients_change;
	SOCKET _maxSock;
	//处理网络消息
	bool OnRun() {
		_clients_change = true;
		while (isRun()) {
			if (_clientsBuff.size() > 0) {
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff) {
					_clients[pClient->sockfd()]=pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
				
			fd_set fdRead;
			
			FD_ZERO(&fdRead);
			if (_clients_change) {
				_clients_change = false;
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter: _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock<iter.second->sockfd()) {
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else {
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}
			
			//文件描述符最大值+1，windows中可以写0
			int ret = select(_maxSock + 1, &fdRead,nullptr, nullptr, nullptr);
			if (ret < 0) {
				printf("select is over\n");
				Close();
				return false;
			}
			
#ifdef _WIN32
			for (int n = 0; n < fdRead.fd_count; n++)
			{
				auto iter = _clients.find(fdRead.fd_array[n]);
				if (iter != _clients.end()) {
					if (-1 == RecvData(iter->second)) {
						if (_pNetEvent) {
							_pNetEvent->OnLeave(iter->second);
							_clients_change = true;
							_clients.erase(iter->first);
						}
					}

				}
				else {
					printf("error iter == _client.end()\n");
				}
			
			}
#else
			std::vector<ClientSocket*> temp;
			for (auto iter : _clients) {
				if (FD_ISSET(iter.second->sockfd(), &fdRead)) {
					if (-1 == RecvData(iter.second)) {
						if (_pNetEvent) {
							_pNetEvent->OnLeave(iter.second);
						}
						_clients_change = false;
						temp.push_back(iter.second);
					}
				}
			}
			for (auto pClient : temp) {
				_clients.erase(pClient->sockfd());
				delete pClient;
			}
#endif // _WIN32
			
		
	
		}
	}
	//关闭socket
	void Close() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				delete iter.second;
			}
			printf("exit \n");
			closesocket(_sock);
			WSACleanup();
#else
			for (auto iter : _clients)
			{
				close(iter.second->sockfd());
				delete iter.second;
			}
			printf("exit \n");
			close(_sock);
#endif
			_clients.clear();
		}
	}
	//是否工作中
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}
	//接收缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	//接受数据 处理粘包 拆分包
	int RecvData(ClientSocket* pClient)
	{
	
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0) {
			//printf("socket-%d client exit\n", pClient->sockfd());
			return -1;
		}
		//将收取到数据拷贝到消息缓冲区0
		memcpy(pClient->msgBuf() + pClient->getLast(), _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLast() + nLen);
		while (pClient->getLast() >= sizeof(DataHeader)) {
			DataHeader *header = (DataHeader*)pClient->msgBuf();
			if (pClient->getLast() >= header->dataLength) {
				//剩余未处理消息缓冲区的长度
				int nSize = pClient->getLast() - header->dataLength;
				OnNetMsg(pClient, header);
				//将未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				pClient->setLastPos(nSize);
			}
			else {
				break;
			}

		}
		return 0;
	}

	//响应网络消息
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader *header) {
		_pNetEvent->OnNetMsg(pClient, header);
		
	}

	////发送指定客户数据
	//int SendData(SOCKET _cSock, DataHeader *header) {
	//	if (isRun() && header) {
	//		return send(_cSock, (const char*)header, header->dataLength, 0);
	//	}
	//	return SOCKET_ERROR;
	//}
};

class EasyTcpServer:public INetEvent
{
private:
	SOCKET _sock;
	//每秒消息计时
	CELLTimestamp _tTime;
	std::vector<CellServer*> _cellServers;
protected:
	//收到消息计数
	std::atomic_int _recvCount;
	//客户端离开计数
	std::atomic_int _leaveCount;
	//客户端进入计数
	std::atomic_int _clientCount;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_leaveCount = 0;
		_clientCount = 0;
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
			addClientToCellServer(new ClientSocket(cSock));
			//获取ip地址 inet_ntoa(clientAddr.sin_addr)
		}
		return cSock;
	}
	void addClientToCellServer(ClientSocket* pClient) {
		//查找客户数量最少的cellserver消息处理
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers) {
			if (pMinServer->getClientCount() > pCellServer->getClientCount()) {
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		OnNetJoin(pClient);
	}
	void Start(int nCellServer) {

		for (int n = 0; n < nCellServer; n++) {
			
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			//注册网络事件接收对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
	}
	//关闭socket
	void Close() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
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
		
		}
	}
	//输出每秒响应网络消息
	void time4msg() {
		auto t1 = _tTime.getElapsedSecond();
		if ( t1>= 1.0) {
			
			printf("thread<%d> time<%lf>,socket<%d>, clients<%d>,recvCount<%d>\n", _cellServers.size(),t1, _sock,(int)_clientCount,int(_recvCount/t1));
			_recvCount = 0;
			_tTime.update();
		}
	
	}
	bool OnRun() {
		if (isRun()) {
			time4msg();
			fd_set fdRead;
			/*fd_set fdWrite;
			fd_set fdExp;*/
			FD_ZERO(&fdRead);
			/*FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);*/
			FD_SET(_sock, &fdRead);
		/*	FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);*/
			
			//文件描述符最大值+1，windows中可以写0
			timeval t = { 0,10 };
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0) {
				printf("select is over\n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;
			}
			return true;
		}
		return false;
	}
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}
	//多线程不安全
	virtual void OnLeave(ClientSocket* pClient) {
		_clientCount--;
	}
	virtual void OnNetMsg(ClientSocket* pClient,DataHeader* header) {
		_recvCount++;
	}
	virtual void OnNetJoin(ClientSocket* pClient) {
		_clientCount++;
	}
};


#endif