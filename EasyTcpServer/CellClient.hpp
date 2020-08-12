#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include "Cell.hpp"
//�ͻ�����������ʱ
#define CLIENT_HEART_DEAD_TIME 60000
//��ʱ���ͻ�����ʱ��
#define CLIENT_SEND_BUFF_TIME 200
//��ſͻ���
class CellClient
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET) {
		static int n = 1;
		id = n++;
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		_lastPos = 0;
		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
		_lastSendPos = 0;
		_dtHeart = 0;
		a = 0;
		resetDTSend();
		//resetDTHeart();
		time.update();
	}
	~CellClient() {
		printf("s = %d CellClient%d.~CellClient\n",serverId, id);
		if (_sockfd != INVALID_SOCKET) {
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
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
	//����ָ���ͻ����� ��ӷ��ͻ�����
	int SendData(netmsg_DataHeader *header) {
		int ret = SOCKET_ERROR;
		int nSendLen = header->dataLength;
		const char* pSendData = (const char*)header;
		while (true) {
			if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE) {
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//����ʣ�����ݵ�λ��
				pSendData += nCopyLen;
				//����ʣ�����ݳ���
				nSendLen -= nCopyLen;
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);
				_lastSendPos = 0;
				resetDTSend();
				if (SOCKET_ERROR == ret) {
					return ret;
				}
			}
			else {
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				_lastSendPos += nSendLen;
				break;
			}
		}
		return ret;
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
	
		//printf("%d\n", dt);
		//printf("%d\n", a);
		a = a + dt;
		_dtHeart = _dtHeart + dt;

		//printf("%d\n", _dtHeart);
		//char* p = (char*)&a;
		//for (int i = 0; i < 8; i++) {_dtHeart
		//	printf("%x", *p);
		//	p--;
		//}
		//printf("\n");
		if (_dtHeart >= CLIENT_HEART_DEAD_TIME) {
			printf("checkHeart dead socket:%d,time=%d\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	//������������
	int SendDataReal(netmsg_DataHeader* header) {
		SendData(header);
		SendDataReal();
	}
	int SendDataReal() {
		int ret = SOCKET_ERROR;
		if (_lastSendPos > 0 && SOCKET_ERROR!=_sockfd) {
			ret = send(_sockfd, _szSendBuf, _lastSendPos, 0);
			_lastSendPos = 0;
			resetDTSend();
		}
		return ret;
	}
	bool checkSend(int dt) {
		_dtSend = _dtSend + dt;
		if (_dtSend >= CLIENT_HEART_DEAD_TIME) {
			//printf("checkSend dead socket:%d,time=%d\n", _sockfd, _dtSend);
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
	char _szMsgBuf[RECV_BUFF_SIZE];
	int _lastPos = 0;
	//���ͻ�����
	char _szSendBuf[SEND_BUFF_SIZE];
	int _lastSendPos = 0;
	//����������ʱ
	int _dtHeart = 0;
	int a = 0;
	/*
	��֪����ʲôbug  ɾ������a�ͻ᲻����ȷִ���߼�
	a�Ĳ����߼���_dtHeart��ͬ������ʹ��a������ȷִ��
	
	*/
	//�ϴη�������ʱ��
	time_t _dtSend;
	public:
		int id = -1;
		int serverId = -1;
};
#endif // !_CellClient_hpp_

