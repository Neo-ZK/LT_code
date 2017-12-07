
#include<stdio.h>

#include "LT_function.h"


char* send(char* arr,int len)
{
	//char buf[000];
	//memcpy(buf,arr,len);
	return arr;
}

int main()
{
	int i;
	//LT_code* m_LT_code = new LT_code("../lena.jpg",0.5,&send);
	//m_LT_code->encodeAndsend();
	char* a = (char*)malloc(2);
	char* b = (char*)malloc(2);
	for (i = 0;i < 2;i++)
	{
		a[i] = 'a';
		b[i] = 'b';
	}
	LT_XOR_char(a,b,2,a);
	LT_XOR_char(a,b,2,a);
	return 0;
}
