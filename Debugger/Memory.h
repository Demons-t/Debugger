#pragma once
#include <Windows.h>

class Memory
{
public:
	int	 ReadMemory(HANDLE hProcess, unsigned int  uAddress, unsigned char* pBuff, int uSize);
	int	 WriteMemory(HANDLE hProcess, unsigned int  uAddress, unsigned char* pBuff, int uSize);
	void ShowMem(SIZE_T sizeAddress, const LPBYTE lpBuff, int nSize);			// ��ʾ�ڴ�
	void EditMem(HANDLE hProcess, SIZE_T sizeAddr, BYTE* pBuff, int nFlags);	// �޸��ڴ�
};

