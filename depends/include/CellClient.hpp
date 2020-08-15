#ifndef _CellClient_hpp_
#define _CellClient_hpp_
#include "CELLBuffer.hpp"
#include "Cell.hpp"
//�ͻ�����������ʱ
#define CLIENT_HEART_DEAD_TIME 60000
//��ʱ���ͻ�����ʱ��
#define CLIENT_SEND_BUFF_TIME 200
//��ſͻ���
class CellClient
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET):_sendBuff(SEND_BUFF_SIZE),_recvBuff(RECV_BUFF_SIZE) {
		static int n = 1;
		id = n++;
		_sockfd = sockfd;
		////
		_dtHeart = 0;
		a = 0;
		/////
		resetDTSend();
		//resetDTHeart();
		time.update();
	}
	~CellClient() {
		CELLLog::Info("s = %d CellClient%d.~CellClient\n",serverId, id);
		if (_sockfd != INVALID_SOCKET) {
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
	}
	bool needWrite() {
		return _sendBuff.needWrite();
	}
	bool hasMsg() {
		return _recvBuff.hasMsg();
	}

	netmsg_DataHeader* front_msg() {
		return (netmsg_DataHeader*)_recvBuff.data();
	}
	void pop_front_msg() {
		if(hasMsg())
		_recvBuff.pop(front_msg()->dataLength);
	}
	SOCKET sockfd() {
		return _sockfd;
	}
	int RecvData() {
		return _recvBuff.read4socket(_sockfd);
	}
	//����ָ���ͻ����� ��ӷ��ͻ�����
	int SendData(netmsg_DataHeader *header) {
		if (_sendBuff.push((const char*)header, header->dataLength)) {
			return header->dataLength;
		}
		return SOCKET_ERROR;
	}

	void resetDTHeart() {
		a = 0;
		_dtHeart = 0;
	}
	void resetDTSend() {
		_dtSend = 0;
	}
	void resetA() {
		_dtHeart = 0;
	}
	//�������
	bool checkHeart(int dt) {
		a = a + dt;
		_dtHeart = _dtHeart + dt;
		if (_dtHeart >= CLIENT_HEART_DEAD_TIME) {
			CELLLog::Info("checkHeart dead socket:%d,time=%d\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	//������������
	void SendDataReal(netmsg_DataHeader* header) {
		SendData(header);
		SendDataReal();
	}
	int SendDataReal() {
		resetDTSend();
		return _sendBuff.write2socket(_sockfd);;
	}
	bool checkSend(int dt) {
		_dtSend = _dtSend + dt;
		if (_dtSend >= CLIENT_HEART_DEAD_TIME) {
			//CELLLog::Info("checkSend dead socket:%d,time=%d\n", _sockfd, _dtSend);
			//ʱ�䵽�������������� ��շ���
			SendDataReal();
			resetDTSend();
			return true;
		}
		return false;
	}
	CELLTimestamp time;
private:
	SOCKET _sockfd;
	//���ջ�����
	CELLBuffer _recvBuff;
	//����������ʱ
	int _dtHeart = 0;
	int a = 0;
	/*
	��֪����ʲôbug  ɾ������a�ͻ᲻����ȷִ���߼�
	a�Ĳ����߼���_dtHeart��ͬ������ʹ��a������ȷִ��
	
	*/
	//���ͻ�����
	CELLBuffer _sendBuff;
	//�ϴη�������ʱ��
	time_t _dtSend;
	//���ͻ�����д���������
	int _sendBuffFullCount = 0;
public:
	int id = -1;
	int serverId = -1;
};
#endif // !_CellClient_hpp_

