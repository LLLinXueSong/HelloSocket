#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include "Cell.hpp"
class CellClient
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		_lastPos = 0;
		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
		_lastSendPos = 0;
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
private:
	SOCKET _sockfd;
	//���ջ�����
	char _szMsgBuf[RECV_BUFF_SIZE];
	int _lastPos = 0;
	//���ͻ�����
	char _szSendBuf[SEND_BUFF_SIZE];
	int _lastSendPos = 0;
};
#endif // !_CellClient_hpp_

