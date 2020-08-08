#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#define MAX_MEMORY_SIZE 64
#include <stdlib.h>
#include <assert.h>

#ifdef _DEBUG
	#include<stdio.h>
	#define xPrintf(...) printf(__VA_ARGS__)
#else
	#define xPrintf(...)                     \\非debug模式下不输出调试信息，替换为空

#endif
class MemoryAlloc;
//内存池中内存块  头部信息
class MemoryBlock {
public:
	//内存块编号
	int nID;
	//引用次数
	int nRef;
	//所属内存池
	MemoryAlloc* pAlloc;
	//下一块位置
	MemoryBlock* pNext;
	//是否在内存池中
	bool bPool;
private:
	//预留补齐
	char c1;
	char c2;
	char c3;
public:
	MemoryBlock() {}
	~MemoryBlock() {}
	
};
//内存池
class MemoryAlloc {
public:
	MemoryAlloc() {
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockSize = 0;
	}
	~MemoryAlloc() {
		if (_pBuf) {
			free(_pBuf);
		}
	}
	void* allocMemory(size_t nSize) {
		if (!_pBuf) {
			initMemory();
		}
		MemoryBlock* pReturn = nullptr;
		if(nullptr == _pHeader){
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else {
			pReturn = _pHeader;
			_pHeader = pReturn->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocMem: %x, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	void freeMemory(void* pMem) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		if (--pBlock->nRef != 0) {
			return;
		}
		if (pBlock->bPool) {
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			free(pBlock);
		}
		
	}
	//初始化
	void initMemory() {
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		//申请池内存
		size_t bufSize = (_nSize+sizeof(MemoryBlock))*_nBlockSize;
		//申请池内存
		_pBuf = (char*)malloc(bufSize);

		//初始化内存池
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		MemoryBlock* pTemp1 = _pHeader;
		size_t realsize = (_nSize + sizeof(MemoryBlock));
		for (size_t n = 1;  n < _nBlockSize; n++) {
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n*realsize));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
protected:
	//内存池地址
	char* _pBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存单元的大小
	size_t _nSize;
	//内存单元的数量
	size_t _nBlockSize;
};
template<size_t nSize,size_t nBlockSize>
class MemoryAlloctor :public MemoryAlloc {
public:
	MemoryAlloctor() {
		const size_t n = sizeof(void*);
		_nSize = (nSize/n)*n +(nSize%n? n:0);
		_nBlockSize = nBlockSize;
	}
};
//内存管理工具
class MemoryMgr {
private:
	MemoryMgr() {
		init(0, 64, &_mem64);
	}
	~MemoryMgr() {}
	MemoryAlloctor<64, 10> _mem64;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
	//初始化内存映射数组
	void init(int nBegin,int nEnd,MemoryAlloc* pMemA) {
		for (int n = nBegin; n <= nEnd; n++) {
			_szAlloc[n] = pMemA;
		}
	}
public:
	//单例模式
	static MemoryMgr& Instance() {
		static MemoryMgr mgr;
		return mgr;
	}
	//
	void* allocMem(size_t nSize) {
		if (nSize <= MAX_MEMORY_SIZE) {
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else {
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			xPrintf("allocMem: %x, id = %d, size = %d \n",pReturn,pReturn->nID, nSize);
			return ((char*)pReturn + sizeof(MemoryBlock));
		}
		
	}
	//增加内存块的引用计数
	void addRef(void* pMem) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
	void freeMem(void* pMem) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		xPrintf("freeMem: %x, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool) {
			pBlock->pAlloc->freeMemory(pMem);
		}
		else {
			if (--pBlock->nRef == 0) {
				free(pBlock);
			}			
		}
		
	}

};
#endif // !_MemoryMgr_hpp_
