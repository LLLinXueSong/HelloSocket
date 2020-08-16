#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#include<stdio.h>
#include<thread>
#include "Cell.hpp"
#include "MessageHeader.hpp"
#include "CELLNetWork.hpp"
#include "CellClient.hpp"
class EasyTcpClient
{
public:
	EasyTcpClient() {
		
		_isConnect = false;
	}
	virtual ~EasyTcpClient() {
		Close();
	}
	//初始化socket
	void initSocket() {
		CELLNetWork::Init();
		//避免重复创建 先关闭
		if (pClient) {
			CELLLog::Info("socket = %d  关闭旧连接。。。。\n",pClient->sockfd());
			Close();
		}
		SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock == INVALID_SOCKET) {
			CELLLog::Info("create socket error\n");
		}
		else {
			pClient = new CellClient(_sock);
			CELLLog::Info("create socket = %d  success\n", pClient->sockfd());
		}
	}
	//连接服务器
	int Connect(const char* ip, short port) {
		if (!pClient) {
			initSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr));
		if (ret == SOCKET_ERROR) {
			CELLLog::Info("socket =%d  port = %d connect error\n", pClient->sockfd(),port);
		}
		else {
			_isConnect = true;
			//CELLLog::Info("socket =%d  port = %d connect success\n", pClient->sockfd(),port);
		}
		return ret;
	}
	//关闭socket
	void Close() {
		if (pClient) { //避免重复关闭
			delete pClient;
			pClient = false;
		}
		_isConnect = false;
	}
	//处理网络消息
	bool OnRun() {
		if (isRun()) {
			SOCKET _sock = pClient->sockfd();
			fd_set fdReads;
			fd_set fdWrite;
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdReads);
			timeval t = { 0,1 };
			int ret = 0;
			FD_SET(_sock, &fdReads);
			if (pClient->needWrite()) {
				
				FD_SET(_sock, &fdWrite);
				ret = select(_sock+1 , &fdReads, &fdWrite, nullptr, &t);
			}
			else {
				ret = select(_sock+1 , &fdReads, nullptr, nullptr, &t);
			}
			if (ret < 0) {
				CELLLog::Info("socket = %d  select is over1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				if (-1 == RecvData(_sock)) {
					CELLLog::Info("socket = %d  select is over2\n", _sock);
					Close();
					return false;
				}
			}
			if (FD_ISSET(_sock, &fdWrite))
			{
				if (-1 == pClient->SendDataReal()) {
					CELLLog::Info("socket = %d  select is over2\n", _sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
	bool isRun() {
		return pClient && _isConnect;
	}
	//接受数据 处理粘包 拆分包
	int RecvData(SOCKET cSock)
	{
		if (isRun()) {
			int nLen = pClient->RecvData();
			if (nLen > 0) {
				while (pClient->hasMsg()) {
					OnNetMsg(pClient->front_msg());
					//移除最前一条数据
					pClient->pop_front_msg();
				}
			}
			//是否有消息处理	
			return nLen;
		}
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(netmsg_DataHeader *header) = 0;
	//发送数据
	int SendData(netmsg_DataHeader *header) {
		if(isRun())
			return pClient->SendData(header);
		return 0;
	}
	int SendData(const char* pData,int len) {
		if (isRun())
			return pClient->SendData(pData,len);
		return 0;
	}
protected:
	CellClient* pClient = nullptr;
	bool _isConnect = false;
};





#endif