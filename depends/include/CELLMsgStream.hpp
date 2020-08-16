#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_
#include "MessageHeader.hpp"
#include "CELLStream.hpp"
//字节流处理
class CELLRecvStream :public CELLStream{
public:
	CELLRecvStream(netmsg_DataHeader* header):CELLStream((char*)header, header->dataLength) {
		push(header->dataLength);
		ReadInt16();
		getNetCmd();
	}
	uint16_t getNetCmd() {
		uint16_t cmd = CMD_ERROR;
		Read<uint16_t>(cmd);
		return cmd;
	}
};
class CELLSendStream :public CELLStream {
public:
	CELLSendStream(char* pData, int nSize, bool bDelete = false) :CELLStream(pData, nSize, bDelete) {
		//预先占领消息长度所需空间
		Write<uint16_t>(0);
	}
	CELLSendStream(int nSize = 1024) :CELLStream(nSize) {
		//预先占领消息长度所需空间
		Write<uint16_t>(0);
	}
	void finish() {
		int pos = length();
		setWritePos(0);
		Write<uint16_t>(pos);
		setWritePos(pos);
	}
	bool WriteString(char* str, int len) {
		return WriteArray(str, strlen(str));
	}
	bool WriteString(const char* str) {
		return WriteArray(str, strlen(str));
	}
	bool WriteString(std::string& str) {
		return WriteArray(str.c_str(), str.length());
	}
	void setNetCmd(uint16_t cmd) {
		Write<uint16_t>(cmd);
	}
};
#endif // _CELL_TASK_H_
