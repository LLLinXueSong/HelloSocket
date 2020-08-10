#ifndef _CELL_HPP_
#define _CELL_HPP_
	class CellServer;
	#include "MessageHeader.hpp"
	#include "CELLTimestamp.hpp"
	#include "CELLTask.hpp"
	#include<stdio.h>
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
#endif // !_CELL_HPP_
