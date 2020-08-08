#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_
#include<thread>
#include<mutex>
#include<list>

	//任务基类
class CellTask {
public:
	CellTask() {}
	virtual ~CellTask() {}
	virtual void doTask() {}
private:
};


class CellTaskServer {
private:
	std::list<CellTask*> _tasks;
	//任务数据缓冲区
	std::list<CellTask*> _tasksBuf;
	std::mutex _mutex;
	
public:
	CellTaskServer() {}
	~CellTaskServer() {}
	void addTask(CellTask* task) {
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);

	}

	void Start(){
		std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
		t.detach();
	}
protected:
	void OnRun() {
		while (true) {
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
				pTask->doTask();
				delete pTask;
			}
			_tasks.clear();
		}
	}
};
#endif // _CELL_TASK_H_
