#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_
#include "Cell.hpp"
#include "CELLTask.hpp"
#include<ctime>
class CELLLog {
	
public:
	static CELLLog& Instance() {
		static CELLLog sLog;
		return sLog;
	}
	//Info
	//Debug
	//Warring
	//Error
	static void Info(const char* pStr) {
		CELLLog* pLog = &Instance();
		pLog->_taskServer.addTask([=](){
			if (pLog->_logFile) {
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s","Info ");
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour + 8, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, "%s", pStr);
				fflush(pLog->_logFile);
			}
			printf("%s", pStr);
		});
	}
	template<typename ...Args>
	static void Info(const char* pformat, Args ... args) {
		CELLLog* pLog = &Instance();
		pLog->_taskServer.addTask([pLog, pformat, args...](){
			if (pLog->_logFile) {
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s", "Info ");
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour + 8, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, pformat, args...);
				fflush(pLog->_logFile);
			}
			printf(pformat, args...);
		});
		
	}
	void setLogPath(const char* logPath,const char* mode) {
		if (_logFile) {
			Info("CELLLog:setLogPath fclose\n");
			fclose(_logFile);
			_logFile = nullptr;
		}
		_logFile = fopen(logPath,mode);
		if (_logFile) {
			Info("CELLLog:setLogPath success,<%s,%s>\n",logPath,mode);
		}
		else {
			Info("CELLLog:setLogPath fail,<%s,%s>\n", logPath, mode);
		}
	}
private:
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;
	CELLLog() {
		_taskServer.Start();
	}
	~CELLLog() {
		_taskServer.Close();
		if (_logFile) {
			Info("CELLLog fclose\n");
			fclose(_logFile);
			_logFile = nullptr;
		}
	}
};
#endif