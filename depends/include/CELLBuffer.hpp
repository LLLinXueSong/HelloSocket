#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_
#include "Cell.hpp"
class CELLBuffer {
public:
	CELLBuffer(int nSize = 8192) {
		_nSize = nSize;
		_pBuff = new char[nSize];
	}
	~CELLBuffer(){
		if (_pBuff) {
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}
	bool push(const char* pData,int nLen) {
		//if (_nLast + nLen > _nSize) {
		//	//д�����ݴ��ڿ��ÿռ� Ҳ����д�����ݿ���ߴ���
		//	int n = _nLast + nLen - _nSize;
		//	if (n < 8192)
		//		n = 8192;
		//	char* buff = new char[_nSize + n];
		//	memcpy(buff, _pBuff, _nLast);
		//	delete[] _pBuff;
		//	_pBuff = buff;
		//}
		if (_nLast + nLen <= _nSize) {
			memcpy(_pBuff + _nLast, pData, nLen);
			_nLast += nLen;
			if (_nLast == SEND_BUFF_SIZE) {
				_BuffFullCount++;
			}
			return true;
		}
		else {
			_BuffFullCount++;
		}
		return false;
	}
	void pop(int nLen) {
		int n = _nLast - nLen;
		if (n > 0) {
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		_nLast = n;
		if (_BuffFullCount > 0)
			--_BuffFullCount;
	}
	int write2socket(SOCKET  sockfd) {
		int ret = 0;
		if (_nLast > 0 && INVALID_SOCKET != sockfd) {
			ret = send(sockfd, _pBuff, _nLast, 0);
			_nLast = 0;
			_BuffFullCount = 0;
		}
		return ret;
	}
	char* data() {
		return _pBuff;
	}
	int read4socket(SOCKET sockfd) {
		if (_nSize - _nLast > 0) {
			char* szRecv = _pBuff + _nLast;
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);
			if (nLen <= 0) {
				//CELLLog::Info("socket-%d client exit\n", pClient->sockfd());
				return nLen;
			}
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}
	bool hasMsg() {
		//��Ϣ������������β��λ�ú���
		if (_nLast >= sizeof(netmsg_DataHeader)) {
			netmsg_DataHeader *header = (netmsg_DataHeader*)_pBuff;
			return _nLast >= header->dataLength;
		}
		return false;
	}
	bool needWrite() {
		return _nLast >0;
	}
private:
	//���ͻ�����
	char* _pBuff = nullptr;
	//������������β��λ�ã��������ݳ���
	int _nLast = 0;
	//�������ܵĿռ��С
	int _nSize;
	//������д������
	int _BuffFullCount = 0;
};
#endif // !_CELL_BUFFER_HPP_
