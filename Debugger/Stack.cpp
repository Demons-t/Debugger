#include "Stack.h"
#include <stdio.h>

void Stack::ShowStatic(HANDLE hProcess, HANDLE hThread)
{
	// ��ʼ���ṹ��
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_CONTROL;

	// ��ȡ��Ŀ���̵߳��̻߳���
	GetThreadContext(hThread, &context);

	BYTE byteBuff[512];
	DWORD dwRead = 0;
	ReadProcessMemory(hProcess, (LPVOID*)context.Esp, byteBuff, 512, &dwRead);
	for (int i = 0; i < 10; i++)
	{
		printf("%08X | %08X\n", context.Esp, ((DWORD*)byteBuff)[i]);
	}
}
