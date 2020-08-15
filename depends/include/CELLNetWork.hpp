#ifndef _CELL_NET_WORK_HPP_
#define _CELL_NET_WORK_HPP_
#include "Cell.hpp"
class CELLNetWork {
public:
	static void Init() {
		static CELLNetWork obj;
	}
private:
	CELLNetWork() {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32
#ifndef _WIN32
		//忽略异常信号，默认情况会导致进程终止
		signal(SIGPIPE, SIG_IGN);
#endif // !_WIN32

	}
	~CELLNetWork() {
#ifdef _WIN32
		WSACleanup();
#endif
	}
};
#endif // !_CELL_NET_WORK_HPP_
