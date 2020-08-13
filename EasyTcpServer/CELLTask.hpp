#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_
#include<thread>
#include<mutex>
#include<list>
#include<functional>
#include "CELLSemaphore.hpp"
//��Ϣ�ظ���
class CellTaskServer {
	//ʹ��function���溯��ָ��   CellTask�ľ��������Ҫ�����������������
	typedef std::function<void()> CellTask;
private:
	std::list<CellTask> _tasks;
	//�������ݻ�����
	std::list<CellTask> _tasksBuf;
	std::mutex _mutex;
	bool _isRun = false;
	CELLSemaphore _sem;
public:
	int _serverId = -1;
	CellTaskServer() {}
	~CellTaskServer() {}
	//����Ϊ����ָ��   addTask���õ�ʱ��ֱ�Ӷ��������������з���
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
			//������������
			if (!_tasksBuf.empty()) {
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf) {
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}
			//û������
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
