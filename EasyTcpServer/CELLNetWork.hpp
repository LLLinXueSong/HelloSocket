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
		//�����쳣�źţ�Ĭ������ᵼ�½�����ֹ
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
