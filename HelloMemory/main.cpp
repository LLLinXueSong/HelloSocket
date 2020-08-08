
#include "Alloctor.h"
#include<stdio.h>
int main() {
	char* data1 = new char[128];
	delete []data1;
	char* data2 = new char;
	delete data2;
	getchar();
	return 0;
}