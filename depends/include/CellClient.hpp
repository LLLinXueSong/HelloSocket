#ifndef _CellClient_hpp_
#define _CellClient_hpp_
#include "CELLBuffer.hpp"
#include "Cell.hpp"
//客户端心跳检测计时
#define CLIENT_HEART_DEAD_TIME 60000
//定时发送缓冲区时间
#define CLIENT_SEND_BUFF_TIME 200
//存放客户端
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
	//发送指定客户数据 添加发送缓冲区
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
	//心跳检测
	bool checkHeart(int dt) {
		a = a + dt;
		_dtHeart = _dtHeart + dt;
		if (_dtHeart >= CLIENT_HEART_DEAD_TIME) {
			CELLLog::Info("checkHeart dead socket:%d,time=%d\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	//立即发送数据
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
			//时间到了立即发送数据 清空发送
			SendDataReal();
			resetDTSend();
			return true;
		}
		return false;
	}
	CELLTimestamp time;
private:
	SOCKET _sockfd;
	//接收缓冲区
	CELLBuffer _recvBuff;
	//心跳死亡计时
	int _dtHeart = 0;
	int a = 0;
	/*
	不知道是什么bug  删除变量a就会不能正确执行逻辑
	a的操作逻辑和_dtHeart相同，但是使用a不能正确执行
	
	*/
	//发送缓冲区
	CELLBuffer _sendBuff;
	//上次发送数据时间
	time_t _dtSend;
	//发送缓冲区写满情况计数
	int _sendBuffFullCount = 0;
public:
	int id = -1;
	int serverId = -1;
};
#endif // !_CellClient_hpp_

