#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
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
#endif
#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 10240
#endif // !RECV_BUFF_SIZE
#include<stdio.h>
#include<thread>
#include "MessageHeader.hpp"
class EasyTcpClient
{
public:
	EasyTcpClient() {
		_sock = INVALID_SOCKET;
		_isConnect = false;
	}
	virtual ~EasyTcpClient() {
		Close();
	}
	//��ʼ��socket
	void initSocket() {
		//����win socket 2.x����
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//�����ظ����� �ȹر�
		if (_sock != INVALID_SOCKET) { 
			printf("socket = %d  �رվ����ӡ�������\n",_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock == INVALID_SOCKET) {
			printf("create socket error\n");
		}
		else {
			//printf("create socket = %d  success\n",_sock);
		}
	}
	//���ӷ�����
	int Connect(const char* ip, short port) {
		if (_sock == INVALID_SOCKET) {
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
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
		if (ret == SOCKET_ERROR) {
			printf("socket =%d  port = %d connect error\n",_sock,port);
		}
		else {
			_isConnect = true;
			//printf("socket =%d  port = %d connect success\n",_sock,port);
		}
		return ret;
	}
	//�ر�socket
	void Close() {
		if (_sock != INVALID_SOCKET) { //�����ظ��ر�
#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
			printf("client exit \n");
			_sock = INVALID_SOCKET;
		}
		_isConnect = false;
	}
	//����������Ϣ
	bool OnRun() {
		if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,1 };
			int ret = select(_sock, &fdReads, 0, 0, &t);
			if (ret < 0) {
				printf("socket = %d  select is over1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock)) {
					printf("socket = %d  select is over2\n", _sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
	bool isRun() {
		return _sock != INVALID_SOCKET && _isConnect;
	}

	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE] = {};
	int _lastPos = 0;
	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET cSock)
	{
		char* szRecv = _szMsgBuf + _lastPos;
		int nLen = recv(cSock, szRecv, (RECV_BUFF_SIZE)-_lastPos, 0);
		netmsg_DataHeader *header = (netmsg_DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("socket-%d client exit", cSock);
			return -1;
		}
		//��Ϣ������������β��λ�ú���
		_lastPos += nLen;
		while (_lastPos >= sizeof(netmsg_DataHeader)) {
			netmsg_DataHeader *header = (netmsg_DataHeader*)_szMsgBuf;
			if (_lastPos >= header->dataLength) {
				//ʣ��δ������Ϣ�������ĳ���
				int nSize = _lastPos - header->dataLength;
				OnNetMsg(header);
				//��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				_lastPos = nSize;
			}
			else {
				break;
			}

		}

		return 0;
	}
	//��Ӧ������Ϣ
	void OnNetMsg(netmsg_DataHeader *header) {
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			netmsg_LoginR *login;
			login = (netmsg_LoginR*)header;
			//printf("<socket=%d>recv server cmd:netmsg_LoginR Len:%d \n",_sock, login->dataLength);
			break;
		}
		case CMD_LOGOUT_RESULT:
		{
			netmsg_LogoutR *logout;
			logout = (netmsg_LogoutR*)header;
			printf("<socket=%d> recv server cmd:netmsg_LoginR Len:%d \n", _sock, logout->dataLength);
			break;
		}
		case CMD_NEW_USER_JOIN:
		{
			netmsg_NewUserJoin *userJoin;
			userJoin = (netmsg_NewUserJoin*)header;
			printf("<socket=%d> new user join cmd:netmsg_LoginR Len:%d \n", _sock,  userJoin->dataLength);
			break;
		}
		case CMD_ERROR:
		{
			printf("<socket=%d> cmd:CMD_ERROR Len:%d \n", header->dataLength);
			break;
		}
		default: {
			printf("<socket=%d> undefine msg, Len:%d \n", _sock, header->dataLength);
		}
		}
	}
	//��������
	int SendData(netmsg_DataHeader *header,int nLen) {
		int ret = SOCKET_ERROR;
		if (isRun() && header) {
			ret = send(_sock, (const char*)header, nLen, 0);
			if (SOCKET_ERROR == ret) {
				Close();
			}
		}
		return ret;
	}
private:
	SOCKET _sock;
	bool _isConnect;
};





#endif