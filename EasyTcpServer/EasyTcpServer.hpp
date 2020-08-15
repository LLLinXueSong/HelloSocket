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
	#define RECV_BUFF_SIZE 10240*10
	#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif // !RECV_BUFF_SIZE


#include<map>
#include<atomic>
#include<thread>

#include<vector>
#include<mutex>
#include "Cell.hpp"
#include "CellClient.hpp"
#include "CellServer.hpp"
#include "INetEvent.hpp"
#include "CELLNetWork.hpp"


class EasyTcpServer:public INetEvent
{
private:
	SOCKET _sock;
	//ÿ����Ϣ��ʱ
	CELLTimestamp _tTime;
	CELLThread _thread;
	std::vector<CellServer*> _cellServers;
protected:
	//�յ���Ϣ����
	std::atomic_int _msgCount;
	//recv����
	std::atomic_int _recvCount;
	//�ͻ����뿪����
	std::atomic_int _leaveCount;
	//�ͻ��˽������
	std::atomic_int _clientCount;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
		_msgCount = 0;
		_recvCount = 0;
		_leaveCount = 0;
		_clientCount = 0;
	}
	virtual ~EasyTcpServer() {
		Close();
	}
	//��ʼ��socket
	SOCKET InitSocket() {
		CELLNetWork::Init();
		//�����ظ����� �ȹر�
		if (_sock != INVALID_SOCKET) {
			CELLLog::Info("socket = %d  �رվ����ӡ�������\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock == INVALID_SOCKET) {
			CELLLog::Info("create socket error\n");
		}
		else {
			CELLLog::Info("create socket = %d  success\n", (int)_sock);
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
			CELLLog::Info("�󶨶˿� <%d> ERROR\n",port);
		}
		else {

			CELLLog::Info("�󶨶˿� <%d> �ɹ�\n",port);
		}
		return ret;
	}
	//�����˿ں�
	int Listen(int n) {
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			CELLLog::Info("socket = %d����ERROR\n",_sock);
		}
		else {

			CELLLog::Info("socket = %d�����ɹ�\n",_sock);
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
			CELLLog::Info("socket = %d ������Ч�ͻ���\n",(int)_sock);
		}
		else {
			addClientToCellServer(new CellClient(cSock));
			//��ȡip��ַ inet_ntoa(clientAddr.sin_addr)
		}
		return cSock;
	}
	void addClientToCellServer(CellClient* pClient) {
		//���ҿͻ��������ٵ�cellserver��Ϣ����
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers) {
			if (pMinServer->getClientCount() > pCellServer->getClientCount()) {
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		
	}
	void Start(int nCellServer) {

		for (int n = 0; n < nCellServer; n++) {
			
			auto ser = new CellServer(n+1);
			_cellServers.push_back(ser);
			//ע�������¼����ն���
			ser->setEventObj(this);
			//������Ϣ�����߳�
			ser->Start();
		}
		_thread.Start(nullptr, [this](CELLThread* pThread) {
			OnRun(pThread);
		});

	}
	//�ر�socket
	void Close() {
		CELLLog::Info("EasyTcpServer.close1 \n");
		_thread.Close();
		if (_sock != INVALID_SOCKET) {
			for (auto s : _cellServers) {
				delete s;
			}
			_cellServers.clear();

			_sock = INVALID_SOCKET;
		}
		CELLLog::Info("EasyTcpServer.close2 \n");
	}
	//���ÿ����Ӧ������Ϣ
	void time4msg() {
		auto t1 = _tTime.getElapsedSecond();
		if ( t1>= 1.0) {
			
			CELLLog::Info("thread<%d> time<%lf>,socket<%d>, clients<%d>,recvCount<%d>,msg<%d> \n", _cellServers.size(),t1, _sock,(int)_clientCount,int(_recvCount/t1),(int)(_msgCount/t1));
			_recvCount = 0;
			_msgCount = 0;
			_tTime.update();
		}
	
	}
	void OnRun(CELLThread* pThread) {
		while (pThread->isRun()) {
			time4msg();
			fd_set fdRead;

			FD_ZERO(&fdRead);

			FD_SET(_sock, &fdRead);

			
			//�ļ����������ֵ+1��windows�п���д0
			timeval t = { 0,1 };
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0) {
				CELLLog::Info("EasyTciServer.OnRun select exit\n");
				pThread->Exit();
				break;
			}
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				Accept();
			}
		}
	}
	//���̲߳���ȫ
	virtual void OnLeave(CellClient* pClient) {
		_clientCount--;
	}
	virtual void OnNetMsg(CellServer* pCellServer,CellClient* pClient,netmsg_DataHeader* header) {
		_msgCount++;
	}
	virtual void OnNetJoin(CellClient* pClient) {
		_clientCount++;
		//CELLLog::Info("client<%d> join\n", pClient->sockfd());
	}
	virtual void OnNetRecv(CellClient* pClient) {
		_recvCount++;

	}
};


#endif