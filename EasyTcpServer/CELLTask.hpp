#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_
#include<thread>
#include<mutex>
#include<list>
#include<functional>
#include "CELLSemaphore.hpp"
//消息回复类
class CellTaskServer {
	//使用function代替函数指针   CellTask的具体操作需要看后边匿名函数定义
	typedef std::function<void()> CellTask;
private:
	std::list<CellTask> _tasks;
	//任务数据缓冲区
	std::list<CellTask> _tasksBuf;
	std::mutex _mutex;
	bool _isRun = false;
	CELLSemaphore _sem;
public:
	int _serverId = -1;
	CellTaskServer() {}
	~CellTaskServer() {}
	//传入为函数指针   addTask调用的时候直接定义匿名函数运行方法
	void addTask(CellTask task) {
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);

	}

	void Start(){
		_isRun = true;
		std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
		t.detach();
	}
	void Close() {
		printf("CellTaskServer%d.close begin \n", _serverId);
		if (_isRun) {	
			_isRun = false;
			_sem.wait();
		}
		printf("CellTaskServer%d.close end \n", _serverId);
	}
protected:
	void OnRun() {
		while (_isRun) {
			//缓冲区有任务
			if (!_tasksBuf.empty()) {
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf) {
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}
			//没有任务
			if (_tasks.empty()) {
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			for (auto pTask : _tasks) {
				pTask();
			}
			_tasks.clear();
		}
		printf("CellTaskServer%d.OnRun exit\n", _serverId);
		_sem.wakeup();
	}
};
#endif // _CELL_TASK_H_
