#pragma once
#include <Windows.h>
#include <atlstr.h>
#include <DbgHelp.h>
#include <fstream>
#include <iomanip>
#include <DbgHelp.h>
#include <TLHELP32.H>
#include<vector>
#include <iostream>
#include <string>
#pragma comment(lib, "Dbghelp.lib")

//// 保存符号信息的结构体
//typedef struct _SYMBOL_INFO
//{
//	ULONG		SizeOfStruct;		// 结构体大小
//	ULONG		TypeIndex;
//	ULONG64		Reserved[2];
//	ULONG		Index;
//	ULONG		Size;
//	ULONG64		ModBase;
//	ULONG		Flags;
//	ULONG64		Value;
//	ULONG64		Address;			// 符号地址
//	ULONG		Register;
//	ULONG		Scope;
//	ULONG		Tag;
//	ULONG		NameLen;			// 符号名的字节数
//	ULONG		MaxNameLen;			// 保存符号名的空间最大字节数
//	TCHAR		Name[1];			// 保存符号名的缓冲区
//}SYMBOL_INFO, *PSYMBOL_INFO;

class Symbol
{
public:
	static std::vector<MODULEENTRY32> vecModules;
	static void		EnumProcessModule(DWORD PID);
	static BOOL		OnInitSym(HANDLE hProcess, CREATE_PROCESS_DEBUG_INFO *info);						// 初始化
	static SIZE_T	GetSymAddress(HANDLE hProcess, const char* pszName);								// 获取符号名
	static SIZE_T	GetSymName(HANDLE hProcess, SIZE_T sizeAddress, CString& strName);					// 获取函数名
	static void		GetSource(HANDLE hPrecess, HANDLE hThread, SIZE_T sizeAddress);						// 获取指定某行的源文件
};

