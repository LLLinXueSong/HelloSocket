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
#define _CellServer_THREAD_COUNT 4
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
private:
	SOCKET _sockfd;
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	int _lastPos = 0;

};
class INetEvent
{
public:
	//�пͻ����뿪ʱ֪ͨ
	virtual void OnLeave(ClientSocket* pClient) = 0;
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header) = 0;
};
class CellServer {
private:
	INetEvent* _pNetEvent;
	std::vector<ClientSocket*> _clientsBuff;
	std::mutex _mutex;
	SOCKET _sock;
	std::thread* _pThread;
	std::vector<ClientSocket*> _clients;
	
public:
	std::atomic_int _recvCount;
public:
	CellServer(SOCKET sock = INVALID_SOCKET) {
		_sock = sock;
		_pThread = nullptr;
		_recvCount = 0;
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
		//thread ʹ�� void (*)(this) mem_fun���Խ�void(CellServer::*)(this) ת��
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
		
	}
	//����������Ϣ
	bool OnRun() {
		while (isRun()) {
			if (_clientsBuff.size() > 0) {
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff) {
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
			}
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
				
			fd_set fdRead;
			
			FD_ZERO(&fdRead);
		
			SOCKET maxSock = _clients[0]->sockfd();
			for (int n = 0; n < _clients.size(); n++)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock<_clients[n]->sockfd()) {
					maxSock = _clients[n]->sockfd();
				}
			}
			//�ļ����������ֵ+1��windows�п���д0
			int ret = select(maxSock + 1, &fdRead,nullptr, nullptr, nullptr);
			if (ret < 0) {
				printf("select is over\n");
				Close();
				return false;
			}

			for (int n = 0; n < _clients.size(); n++)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead)) {
					if (-1 == RecvData(_clients[n])) {
						std::vector<ClientSocket*>::iterator iter = _clients.begin() + n;
						if (iter != _clients.end()) {
							if (_pNetEvent) {
								_pNetEvent->OnLeave(_clients[n]);
							}
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
		}
	}
	//�ر�socket
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
	//�Ƿ�����
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}
	//���ջ�����
	char _szRecv[RECV_BUFF_SIZE] = {};
	//�������� ����ճ�� ��ְ�
	int RecvData(ClientSocket* pClient)
	{
	
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0) {
			printf("socket-%d client exit\n", pClient->sockfd());
			return -1;
		}
		//����ȡ�����ݿ�������Ϣ������0
		memcpy(pClient->msgBuf() + pClient->getLast(), _szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLast() + nLen);
		while (pClient->getLast() >= sizeof(DataHeader)) {
			DataHeader *header = (DataHeader*)pClient->msgBuf();
			if (pClient->getLast() >= header->dataLength) {
				//ʣ��δ������Ϣ�������ĳ���
				int nSize = pClient->getLast() - header->dataLength;
				OnNetMsg(pClient->sockfd(), header);
				//��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				pClient->setLastPos(nSize);
			}
			else {
				break;
			}

		}

		return 0;
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET cSock, DataHeader *header) {
		_recvCount++;
		//_pNetEvent->OnNetMsg(cSock, header);
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


	//����ָ���ͻ�����
	int SendData(SOCKET _cSock, DataHeader *header) {
		if (isRun() && header) {
			return send(_cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//Ⱥ������
	void SendDataToAll(DataHeader *header) {
		for (int n = 0; n < _clients.size(); n++)
		{
			SendData(_clients[n]->sockfd(), header);
		}

	}
};

class EasyTcpServer:public INetEvent
{
private:
	SOCKET _sock;
	CELLTimestamp _tTime;
	std::vector<ClientSocket*> _clients;
	std::vector<CellServer*> _cellServers;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer() {
		Close();
	}
	//��ʼ��socket
	SOCKET InitSocket() {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32
		//�����ظ����� �ȹر�
		if (_sock != INVALID_SOCKET) {
			printf("socket = %d  �رվ����ӡ�������\n", (int)_sock);
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
	//�󶨶˿ں�
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
			printf("�󶨶˿� <%d> ERROR\n",port);
		}
		else {

			printf("�󶨶˿� <%d> �ɹ�\n",port);
		}
		return ret;
	}
	//�����˿ں�
	int Listen(int n) {
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("socket = %d����ERROR\n",_sock);
		}
		else {

			printf("socket = %d�����ɹ�\n",_sock);
		}
		return ret;
	}
	//���տͻ�����
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
			printf("socket = %d ������Ч�ͻ���\n",(int)_sock);
		}
		else {
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			//printf("socket=<%d> �¿ͻ��˼���socket = %d IP:%s \n", (int)_sock,(int)cSock, inet_ntoa(clientAddr.sin_addr));
			
			addClientToCellServer(new ClientSocket(cSock));
		}
		return cSock;
	}
	void addClientToCellServer(ClientSocket* pClient) {
		_clients.push_back(pClient);
		//���ҿͻ��������ٵ�cellserver��Ϣ����
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers) {
			if (pMinServer->getClientCount() > pCellServer->getClientCount()) {
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
	}
	void Start() {

		for (int n = 0; n < 4; n++) {
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->setEventObj(this);
			ser->Start();
		}
	}
	//�ر�socket
	void Close() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = 0; n <_CellServer_THREAD_COUNT; n++)
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
	//��Ӧ������Ϣ
	void time4msg() {
		auto t1 = _tTime.getElapsedSecond();
		if ( t1>= 1.0) {
			int recvCount = 0;
			for (auto ser : _cellServers)
			{
				recvCount += ser->_recvCount;
				ser->_recvCount = 0;
			}
			printf("thread<%d> time<%lf>,socket<%d>, clients<%d>,recvCount<%d>\n", _cellServers.size(),t1, _sock,_clients.size(),int(recvCount/t1));
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
			
			//�ļ����������ֵ+1��windows�п���д0
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
	//����ָ���ͻ�����
	int SendData(SOCKET _cSock,DataHeader *header) {
		if (isRun() && header) {
			return send(_cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//Ⱥ������
	void SendDataToAll(DataHeader *header) {
		for (int n = 0; n < _clients.size(); n++)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}
	virtual void OnLeave(ClientSocket* pClient) {
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			if (_clients[n] == pClient) {
				auto iter = _clients.begin() + n;
				if (iter != _clients.end()) {
					_clients.erase(iter);
				}
			}
		}
	}
	virtual void OnNetMsg(SOCKET cSock,DataHeader* header) {
		//time4msg();
	}
};


#endif