#ifndef CELLSERVER_HPP_
#define CELLSERVER_HPP_
#include "Cell.hpp"
#include "INetEvent.hpp"
#include <map>
#include <vector>
#include "CELLSemaphore.hpp"
//������Ϣ���մ��������
class CellServer {
private:
	//�����¼�����
	INetEvent* _pNetEvent;
	std::vector<CellClient*> _clientsBuff;
	//���������
	std::mutex _mutex;
	std::map<SOCKET, CellClient*> _clients;
	CellTaskServer _taskServer;
	//����socket fd_set
	fd_set _fdRead_bak;
	//�ͻ��Ƿ����ı�
	bool _clients_change = true;
	SOCKET _maxSock;
	time_t _oldTime = CELLTime::getNowInMilliSec();
	int _id = -1;
	CELLThread _thread;
public:
	CellServer(int id) {
		_id = id;
		_pNetEvent = nullptr;
		_taskServer._serverId = id;
	}
	~CellServer() {
		printf("CellServer%d.~CellServer  close1 begin\n", _id);
		Close();
		printf("CellServer%d.~CellServer  close2 end\n", _id);
	}
	//void addSendTask(CellClient* pClient, netmsg_DataHeader* header) {
	//	//�����ڻ�������ֱ�ӵ��ú���ָ����ָ��ĺ����������±߶������������  ����Ҫ������ֱ��ִ�з������ݵ���� �����ǵ��ú��������
	//	/*
	//		for (auto pTask : _tasks) {
	//			pTask();
	//		}
	//	*/
	//	_taskServer.addTask([pClient, header]() {
	//		pClient->SendData(header);
	//		delete header;
	//	});
	//}
	void addClient(CellClient* pClient) {
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
		//���������߳�
		_taskServer.Start();
		_thread.Start(nullptr, [this](CELLThread* pThread) {
			OnRun(pThread);
		},
			[this](CELLThread* pThread) {
			ClearClients();
		});

	}

	//����������Ϣ
	bool OnRun(CELLThread* pThread) {
		while (pThread->isRun()) {
			if (_clientsBuff.size() > 0) {
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff) {
					_clients[pClient->sockfd()] = pClient;
					pClient->serverId = _id;
					if (_pNetEvent) {
						_pNetEvent->OnNetJoin(pClient);
					}
					
					
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			if (_clients.empty())
			{
				//��������ʱ���
				_oldTime = CELLTime::getNowInMilliSec();
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			fd_set fdRead;
			fd_set fdWrite;
		//	fd_set fdExc;

			
			if (_clients_change) {
				_clients_change = false;
				FD_ZERO(&fdRead);
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
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
			memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));
		//	memcpy(&fdExc, &_fdRead_bak, sizeof(fd_set));
			timeval t{ 0,1 };
			//�ļ����������ֵ+1��windows�п���д0
			int ret = select(_maxSock + 1, &fdRead, &fdWrite,nullptr, &t);
			if (ret < 0) {
				printf("CELLServer%d.OnRun.select Error\n",_id);
				pThread->Exit();
				return false;
			}
			//else if (ret == 0) {
			//	continue;
			//}

			ReadData(fdRead);
			WriteData(fdWrite);
		//	WriteData(fdExc);
			CheckTime();

		}
		printf("CellServer%d.OnRun  exit\n", _id);

	}
	void OnClientLeave(CellClient* pClient) {
		if (_pNetEvent) {
			_pNetEvent->OnLeave(pClient);
		}
		_clients_change = true;
		delete pClient;
	}
	void WriteData(fd_set& fdWrite) {
#ifdef _WIN32
		for (int n = 0; n < fdWrite.fd_count; n++)
		{
			auto iter = _clients.find(fdWrite.fd_array[n]);
			if (iter != _clients.end()) {
				if (-1 == iter->second->SendDataReal()) {
						OnClientLeave(iter->second);
						_clients.erase(iter->first);
					}
			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end();) {
			if (FD_ISSET(iter.second->sockfd(), &fdRead)) {
				if (-1 == RecvData(iter.second)) {
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(pClient->sockfd());
					continue;
				}
			}
			iter++;
		}
#endif // _WIN32
	}
	void CheckTime() {
		auto nowTime = CELLTime::getNowInMilliSec();
		int dt = int(nowTime - _oldTime);
		_oldTime = nowTime;
		for (auto iter = _clients.begin(); iter != _clients.end();) {
			if (iter->second->checkHeart(dt)) {
				if (_pNetEvent) {
					_pNetEvent->OnLeave(iter->second);
				}
				_clients_change = true;
			
				delete iter->second;
				auto iterOld = iter;
				iter++;
				_clients.erase(iterOld);
				continue;
			}
			//iter->second->checkSend(dt);
			iter++;
		}
	}
	void ReadData(fd_set& fdRead) {
#ifdef _WIN32
		for (int n = 0; n < fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end()) {
				if (-1 == RecvData(iter->second)) {
					OnClientLeave(iter->second);
						_clients.erase(iter->first);
					}
			}
			else {
				printf("error iter == _client.end()\n");
			}

		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end();) {
			if (FD_ISSET(iter.second->sockfd(), &fdRead)) {
				if (-1 == RecvData(iter.second)) {
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(pClient->sockfd());
					continue;
				}
			}
			iter++;
		}
#endif // _WIN32
	}
	//�ر�socket
	void Close() {
		printf("CellServer%d.Close  close1 begin\n", _id);
		_taskServer.Close();
		_thread.Close();
		printf("CellServer%d.Close  close1 end\n", _id);
	}
	void ClearClients() {
		for (auto iter : _clients)
		{
			delete iter.second;
		}
		_clients.clear();
		for (auto iter : _clientsBuff) {
			delete iter;
		}
		_clientsBuff.clear();
	}
	//�������� ����ճ�� ��ְ�
	int RecvData(CellClient* pClient)
	{
		char* szRecv = pClient->msgBuf() + pClient->getLast();
		int nLen = (int)recv(pClient->sockfd(), szRecv, (RECV_BUFF_SIZE)-pClient->getLast(), 0);
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0) {
			//printf("socket-%d client exit\n", pClient->sockfd());
			return -1;
		}
		//��Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLast() + nLen);
		while (pClient->getLast() >= sizeof(netmsg_DataHeader)) {
			netmsg_DataHeader *header = (netmsg_DataHeader*)pClient->msgBuf();
			if (pClient->getLast() >= header->dataLength) {
				//ʣ��δ������Ϣ�������ĳ���
				int nSize = pClient->getLast() - header->dataLength;
				OnNetMsg(pClient, header);
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
	virtual void OnNetMsg(CellClient* pClient, netmsg_DataHeader *header) {
		_pNetEvent->OnNetMsg(this, pClient, header);

	}

	////����ָ���ͻ�����
	//int SendData(SOCKET _cSock, netmsg_DataHeader *header) {
	//	if (isRun() && header) {
	//		return send(_cSock, (const char*)header, header->dataLength, 0);
	//	}
	//	return SOCKET_ERROR;
	//}
};
#endif // !CELLSERVER_HPP_
