#ifndef _CELL_STREAM_HPP_
#define _CELL_STREAM_HPP_
#include "celllog.hpp"
#include <cstdint>
//字节流处理
class CELLStream {
public:
	CELLStream(char* pData,int nSize,bool bDelete = false) {
		_nSize = nSize;
		_pBuff = pData;
		_bDelete = bDelete;
	}
	virtual ~CELLStream() {
		if (_bDelete && _pBuff) {
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}
	CELLStream(int nSize = 1024) {
		_nSize = nSize;
		_pBuff = new char[nSize];
		_bDelete = true;
	}
	char* data() {
		return _pBuff;
	}
	int length() {
		return _nWritePos;
	}
	inline void setWritePos(int n) {
		_nWritePos = n;
	}
	inline int getWritePos() {
		return _nWritePos;
	}
	inline bool canRead(int n) {
		return _nSize - _nReadPos >= n;
	}
	inline bool canWrite(int n) {
		return _nSize - _nWritePos >= n;
	}
	inline void push(int n) {
		_nWritePos += n;
	}
	inline void pop(int n) {
		_nReadPos += n;
	}
	template<class T>
	bool Read(T& n,bool boffset = true) {
		auto nLen = sizeof(T);
		if (canRead(nLen)) {
			memcpy(&n, _pBuff + _nReadPos, nLen);
			if(boffset)
				pop(nLen);
			return true;
		}
		CELLLog::Info("CELLStream::Read error\n");
		return false;
	}
	template<class T>
	bool onlyRead(T& n) {
		return Read(n, false);
	}
	template<class T>
	uint32_t ReadArray(T* pArr,uint32_t len) {
		uint32_t len1 = 0;
		Read(len1,false);
		if (len1 < len) {
			auto nLen = len1 * sizeof(T);
			if ( canRead(nLen + sizeof(uint32_t)) ) {
				pop(sizeof(uint32_t));
				memcpy(pArr, _pBuff + _nReadPos, nLen);
				pop(nLen);
				return len1;
			}
		}
		CELLLog::Info("CELLStream::ReadArray error\n");
		return 0;
	}
	int8_t ReadInt8() {
		int8_t n = 0;
		Read(n);
		return n;
	}
	int16_t ReadInt16() {
		int16_t n = 0;
		Read(n);
		return n;
	}
	int32_t ReadInt32() {
		int32_t n = 0;
		Read(n);
		return n;
	}
	float ReadFloat() {
		float n = 0;
		Read(n);
		return n;
	}
	double ReadDouble() {
		double n = 0;
		Read(n);
		return n;
	}

	template<typename T>
	bool Write(T n) {
		auto nLen = sizeof(T);
		if (canWrite(nLen)) {
			memcpy(_pBuff + _nWritePos, &n, nLen);
			push(nLen);
			return true;
		}
		CELLLog::Info("CELLStream::Write error\n");
		return false;
	}
	template<class T>
	bool WriteArray(T* pData, uint32_t len) {
		//接收端不知道收到的长度
		
		auto nLen = sizeof(T)*len;
		if (canWrite(nLen + sizeof(uint32_t))) {
			Write(len);
			memcpy(_pBuff + _nWritePos, pData, nLen);
			push(nLen);
			return true;
		}
		CELLLog::Info("CELLStream::WriteArray error\n");
		return false;
	}
	bool WriteInt8(int8_t n) {
		return Write(n);
	}
	bool WriteInt16(int16_t n) {
		return Write(n);
	}
	bool WriteInt32(int32_t n) {
		return Write(n);
	}
	bool WriteFloat(float n) {
		return Write(n);
	}
	bool WriteDouble(double n) {
		return Write(n);
	}
private:
	//发送缓冲区
	char* _pBuff = nullptr;
	//已经写入位置
	int _nWritePos = 0;
	//已经读取数据位置
	int _nReadPos = 0;
	//缓冲区总的空间大小
	int _nSize;
	//pBuff是外部传入数据块时是否释放
	bool _bDelete = true;
};
#endif // _CELL_TASK_H_
