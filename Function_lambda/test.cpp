#include<stdio.h>
#include<functional>
int funcA(int a, int b) {
	return 0;
}
int main() {
	std::function<int(int, int)> call = funcA;
	int n = call(0, 2);

	//匿名函数				可以使用外部变量，必须先在列表里捕获
	call = [n/*外部变量捕获列表*/](int a,int b/*参数列表*/) -> int /*返回值类型*/{ 
		//函数体
		printf("%d\n",n);
		return 2;
	};
	call(5,6);
}