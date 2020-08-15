#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_
#include<thread>
#include<mutex>
#include<list>
#include<functional>
#include "CELLThread.hpp"
//��Ϣ�ظ���
class CellTaskServer {
	//ʹ��function���溯��ָ��   CellTask�ľ��������Ҫ�����������������
	typedef std::function<void()> CellTask;
private:
	std::list<CellTask> _tasks;
	//�������ݻ�����
	std::list<CellTask> _tasksBuf;
	std::mutex _mutex;
	CELLThread _thread;
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
		for (auto pTask : _tasksBuf) {
			pTask();
		}
		//CELLLog::Info("CellTaskServer%d.OnRun exit\n", _serverId);
	}
};
#endif // _CELL_TASK_H_
