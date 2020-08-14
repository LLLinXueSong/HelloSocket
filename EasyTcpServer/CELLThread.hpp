#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_
#include "CELLSemaphore.hpp"
class CELLThread {
private:
	bool _isRun = false;
	//控制线程终止 退出
	CELLSemaphore _sem;
	typedef std::function<void(CELLThread*)> EventCall;
	EventCall _onCreate;
	EventCall _onRun;
	EventCall _onDestory;
	//不同线程中改变数据需要加锁
	std::mutex _mutex;
public:
	void Start(EventCall onCreate = nullptr,
		EventCall onRun = nullptr,
		EventCall onDestory = nullptr) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun) {
			if (onCreate) {
				_onCreate = onCreate;
			}
			if (onRun) {
				_onRun = onRun;
			}
			if (onDestory) {
				_onDestory = onDestory;
			}
			_isRun = true;
			std::thread t(std::mem_fn(&CELLThread::OnWork), this);
			t.detach();
		}

	}
	bool isRun() {
		return _isRun;
	}
	//在工作运行中退出 不需要使用信号量阻塞等待
	void Exit() {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun) {
			_isRun = false;
		}
	}
	//其他线程调用退出
	void Close() {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun) {
			_isRun = false;
			_sem.wait();
		}
	}
protected:
	void OnWork() {
		if (_onCreate)
			_onCreate(this);
		if (_onRun)
			_onRun(this);
		if (_onDestory)
			_onDestory(this);
		_sem.wakeup();
	}
};
#endif // !_CELL_THREAD_HPP_
