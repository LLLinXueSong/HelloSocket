#include<stdio.h>
#include<functional>
int funcA(int a, int b) {
	return 0;
}
int main() {
	std::function<int(int, int)> call = funcA;
	int n = call(0, 2);

	//��������				����ʹ���ⲿ���������������б��ﲶ��
	call = [n/*�ⲿ���������б�*/](int a,int b/*�����б�*/) -> int /*����ֵ����*/{ 
		//������
		printf("%d\n",n);
		return 2;
	};
	call(5,6);
}