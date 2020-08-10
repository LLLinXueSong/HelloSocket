#ifndef _CELLObjectPool_hpp_
#define _CELLObjectPool_hpp_
#include <mutex>
#include <assert.h>
#ifdef _DEBUG
	#ifndef xPrintf
		#include <stdio.h>
		#define xPrintf(...) printf(__VA_ARGS__)
	#endif // !xPrintf
#else
	#ifndef xPrintf
		#define xPrintf(...)                     \\非debug模式下不输出调试信息，替换为空
	#endif
#endif
template<class Type, size_t nPoolSize>
class CELLObjectPool;
template<class Type,size_t nPoolSize>
class ObjectPoolBase {
public:
	void* operator new(size_t nSize) {
		return objectPool().allocObjMemory(nSize);
	}
	void operator delete(void* p) {
		objectPool().freeObjMemory(p);
	}
	//模板的不定参数
	template<typename ...Args> 
	static Type* createObject(Args ... args) {
		Type* obj = new Type(args...);
		return obj;
	}

	static void destroyObject(Type* obj) {
		delete obj;
	}
private:
	//
	typedef   CELLObjectPool<Type, nPoolSize>  ClassTypePool;
	//
	static ClassTypePool& objectPool() {
		static ClassTypePool sPool;
		return sPool;
	}
};
template<class Type,size_t nPoolSize>
class CELLObjectPool {
private:
	struct NodeHeader {
		//下一块位置
		NodeHeader* pNext;
		//编号
		int nID;
		//引用次数
		char nRef;
		//是否在池中
		bool bPool;
	private:
		//预留字节
		char c1;
		char c2;
	};
	NodeHeader* _pHeader;
	//对象池地址
	char* _pBuf;
	std::mutex _mutex;
public:
	CELLObjectPool() {
		initPool();
	}
	~CELLObjectPool() {
		delete[] _pBuf;
	}
	//释放对象
	void freeObjMemory(void* pMem) {    //pMem是需要释放的，使用的内存块地址           每个内存池中结构是 头结点加上申请使用的节点
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
		assert(1 == pBlock->nRef);
		if (pBlock->bPool) {
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0) {
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			if (--pBlock->nRef != 0) {
				return;
			}
			delete[] pBlock;
		}

	}
	//申请对象
	void* allocObjMemory(size_t nSize) {
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader* pReturn = nullptr;
		if (nullptr == _pHeader) {
			pReturn = (NodeHeader*)new char[(sizeof(Type) + sizeof(NodeHeader))];
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else {
			pReturn = _pHeader;
			_pHeader = pReturn->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocObjMem: %x, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(NodeHeader));
	}
	
	//初始化对象池
	void initPool() {
		assert(nullptr == _pBuf);
		size_t n = nPoolSize*(sizeof(Type) + sizeof(NodeHeader));
		_pBuf = new char[n];

		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		NodeHeader* pTemp1 = _pHeader;
		size_t realsize = sizeof(Type) + sizeof(NodeHeader);
		for (size_t n = 1; n < nPoolSize; n++) {
			NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + (n*realsize));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}

};

#endif // !_CELLObjectPool_hpp_
