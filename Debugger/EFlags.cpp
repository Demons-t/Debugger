#include "EFlags.h"
#include <stdio.h>

void EFlags::ShowRegisters(HANDLE hThread)
{
	// 初始化结构体
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_ALL;

	// 获取到目标线程的线程环境
	GetThreadContext(hThread, &context);

	printf("EAX：%08X\n", context.Eax);
	printf("ECX：%08X\n", context.Ecx);
	printf("EDX：%08X\n", context.Edx);
	printf("EBX：%08X\n", context.Ebx);
	printf("ESP：%08X\n", context.Esp);
	printf("EBP：%08X\n", context.Ebp);
	printf("ESI：%08X\n", context.Esi);
	printf("EDI：%08X\n", context.Edi);
	printf("\n");
	printf("EIP：%08X\n", context.Eip);
	printf("\n");

	PEFLAGS pEFlgas = (PEFLAGS)&context.EFlags;
	printf("CF：%05d\n", pEFlgas->CF);
	printf("PF：%05d\n", pEFlgas->PF);
	printf("AF：%05d\n", pEFlgas->AF);
	printf("ZF：%05d\n", pEFlgas->ZF);
	printf("SF：%05d\n", pEFlgas->SF);
	printf("TF：%05d\n", pEFlgas->TF);
	printf("DF：%05d\n", pEFlgas->DF);
	printf("OF：%05d\n", pEFlgas->OF);
}

void EFlags::MemRegisters(HANDLE hThread, char* eFlags, DWORD dwNum)
{
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_ALL;
	GetThreadContext(hThread, &context);
	if (!strcmp(eFlags, "EDI")) { context.Edi = dwNum; }
	else if (!strcmp(eFlags, "ESI")) { context.Esi = dwNum; }
	else if (!strcmp(eFlags, "EBX")) { context.Ebx = dwNum; }
	else if (!strcmp(eFlags, "EDX")) { context.Edx = dwNum; }
	else if (!strcmp(eFlags, "ECX")) { context.Ecx = dwNum; }
	else if (!strcmp(eFlags, "EAX")) { context.Eax = dwNum; }
	else if (!strcmp(eFlags, "EBP")) { context.Ebp = dwNum; }
	else if (!strcmp(eFlags, "EIP")) { context.Eip = dwNum; }
	else if (!strcmp(eFlags, "EFLAGS")) { context.EFlags = dwNum; }
	else if (!strcmp(eFlags, "ESP")) { context.Esp = dwNum; }
	else { printf("没有这个寄存器\n"); }
	SetThreadContext(hThread, &context);
}

void EFlags::MemEFlagsRegisters(HANDLE hThread, char * eFlags, DWORD dwNum)
{
	// 初始化结构体
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_ALL;

	// 获取到目标线程的线程环境
	GetThreadContext(hThread, &context);

	PEFLAGS pEFlgas = (PEFLAGS)&context.EFlags;
	if (!strcmp(eFlags, "CF")) { pEFlgas->CF = dwNum; }
	else if (!strcmp(eFlags, "PF")) { pEFlgas->CF = dwNum; }
	else if (!strcmp(eFlags, "AF")) { pEFlgas->PF = dwNum; }
	else if (!strcmp(eFlags, "ZF")) { pEFlgas->AF = dwNum; }
	else if (!strcmp(eFlags, "SF")) { pEFlgas->ZF = dwNum; }
	else if (!strcmp(eFlags, "TF")) { pEFlgas->SF = dwNum; }
	else if (!strcmp(eFlags, "DF")) { pEFlgas->TF = dwNum; }
	else if (!strcmp(eFlags, "OF")) { pEFlgas->DF = dwNum; }
	else { printf("没有这个寄存器\n"); }
	SetThreadContext(hThread, &context);
}
