#pragma once
#include <Windows.h>
#include <stdio.h>

class PE
{
private:
	static	PIMAGE_DOS_HEADER	m_pDos;
	static	PIMAGE_NT_HEADERS	m_pNt;

public:
	static	DWORD	RvaToFoa(char* pFileData, DWORD rva);
	static	char*	OpenFile(PCSTR lpFilePath);
	static	void	EnumImport(char *pBuff);			// ���������
	static	void	EnumExport(char* pBuff);			// ����������
	//static	void	ReadMemoryBytes(HANDLE hProcess, DWORD nAddress, LPBYTE nValue, DWORD nLen);
	//static	DWORD	GetImageBassAddress(HANDLE hProcess);
	//static	LPWSTR	GetRoute(const wchar_t* type, TCHAR nFileName);
	//static	void	DumpFile(HANDLE hProcess);
	static	void	Dump(const char* pName, HANDLE hProcess);
};

