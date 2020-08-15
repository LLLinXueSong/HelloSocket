#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_
#include<thread>
#include<mutex>
#include<list>
#include<functional>
#include "CELLThread.hpp"
//消息回复类
class CellTaskServer {
	//使用function代替函数指针   CellTask的具体操作需要看后边匿名函数定义
	typedef std::function<void()> CellTask;
private:
	std::list<CellTask> _tasks;
	//任务数据缓冲区
	std::list<CellTask> _tasksBuf;
	std::mutex _mutex;
	CELLThread _thread;
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
		_thread.Start(nullptr, [this](CELLThread* pThread) {
			OnRun(pThread);
		});
	}
	void Close() {
		//CELLLog::Info("CellTaskServer%d.close begin \n", _serverId);
		_thread.Close();
		//CELLLog::Info("CellTaskServer%d.close end \n", _serverId);
	}
protected:
	void OnRun(CELLThread* pThread) {
		while (pThread->isRun()) {
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
		for (auto pTask : _tasksBuf) {
			pTask();
		}
		//CELLLog::Info("CellTaskServer%d.OnRun exit\n", _serverId);
	}
};
#endif // _CELL_TASK_H_
