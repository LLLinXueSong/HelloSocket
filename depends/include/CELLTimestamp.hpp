#ifndef _CELLTimestamp_hpp_
#define CELLTimestamp_hpp_

#pragma once
#include<chrono>
using namespace std::chrono;
class CELLTime {
public:
	//��ȡ��ǰʱ��� ����
	static time_t getNowInMilliSec() {
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}
};
class CELLTimestamp {
public:
	CELLTimestamp() {
		update();
	}
	//����ʱ��
	void update() {
		_begin = high_resolution_clock::now();
	}
	//��ȡ��ǰ��
	double getElapsedSecond() {
		return getElapsedTimeInMicroSec() * 0.000001;
	}
	//��ȡ��ǰ����
	double getElapsedTimeInMilliSec() {
		return this->getElapsedTimeInMicroSec() * 0.001;
	}
	//��ȡ΢��
	long long getElapsedTimeInMicroSec() {
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}
protected:
	time_point<high_resolution_clock> _begin;
};


#endif // !_CELLTimestamp_hpp_