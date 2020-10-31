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

//// ���������Ϣ�Ľṹ��
//typedef struct _SYMBOL_INFO
//{
//	ULONG		SizeOfStruct;		// �ṹ���С
//	ULONG		TypeIndex;
//	ULONG64		Reserved[2];
//	ULONG		Index;
//	ULONG		Size;
//	ULONG64		ModBase;
//	ULONG		Flags;
//	ULONG64		Value;
//	ULONG64		Address;			// ���ŵ�ַ
//	ULONG		Register;
//	ULONG		Scope;
//	ULONG		Tag;
//	ULONG		NameLen;			// ���������ֽ���
//	ULONG		MaxNameLen;			// ����������Ŀռ�����ֽ���
//	TCHAR		Name[1];			// ����������Ļ�����
//}SYMBOL_INFO, *PSYMBOL_INFO;

class Symbol
{
public:
	static std::vector<MODULEENTRY32> vecModules;
	static void		EnumProcessModule(DWORD PID);
	static BOOL		OnInitSym(HANDLE hProcess, CREATE_PROCESS_DEBUG_INFO *info);						// ��ʼ��
	static SIZE_T	GetSymAddress(HANDLE hProcess, const char* pszName);								// ��ȡ������
	static SIZE_T	GetSymName(HANDLE hProcess, SIZE_T sizeAddress, CString& strName);					// ��ȡ������
	static void		GetSource(HANDLE hPrecess, HANDLE hThread, SIZE_T sizeAddress);						// ��ȡָ��ĳ�е�Դ�ļ�
};

