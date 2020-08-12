#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include "Cell.hpp"
//客户端心跳检测计时
#define CLIENT_HEART_DEAD_TIME 5000
class CellClient
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		_lastPos = 0;
		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
		_lastSendPos = 0;
		_dtHeart = 0;
		a = 0;
		//resetDTHeart();
		time.update();
	}
	~CellClient() {
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
	//发送指定客户数据 添加发送缓冲区
	int SendData(netmsg_DataHeader *header) {
		int ret = SOCKET_ERROR;
		int nSendLen = header->dataLength;
		const char* pSendData = (const char*)header;
		while (true) {
			if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE) {
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//计算剩余数据的位置
				pSendData += nCopyLen;
				//计算剩余数据长度
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

	void resetDTHeart() {
		a = 0;
	}
	void resetA() {
		_dtHeart = 0;
	}
	//心跳检测
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
	CELLTimestamp time;
private:
	SOCKET _sockfd;
	//接收缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE];
	int _lastPos = 0;
	//发送缓冲区
	char _szSendBuf[SEND_BUFF_SIZE];
	int _lastSendPos = 0;
	//心跳死亡计时
	int _dtHeart = 0;
	int a = 0;
	/*
	不知道是什么bug  删除变量a就会不能正确执行逻辑
	a的操作逻辑和_dtHeart相同，但是使用a不能正确执行
	
	*/
	
};
#endif // !_CellClient_hpp_

