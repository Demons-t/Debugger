#include "Memory.h"
#include "Memory.h"
#include <stdio.h>
#include "Capstone.h"

int Memory::ReadMemory(HANDLE hProcess, unsigned int  uAddress, unsigned char* pBuff, int uSize)
{
	DWORD	dwRead = 0;
	ReadProcessMemory(hProcess, (LPVOID)uAddress, pBuff, uSize, &dwRead);
	return dwRead;
}

int Memory::WriteMemory(HANDLE hProcess, unsigned int uAddress, unsigned char * pBuff, int uSize)
{
	DWORD	dwRead = 0;
	WriteProcessMemory(hProcess, (LPVOID)uAddress, pBuff, uSize, &dwRead);
	return dwRead;
}

void Memory::ShowMem(SIZE_T sizeAddress, const LPBYTE lpBuff, int nSize)
{
	unsigned char ch = 0;
	const char* pAnsi = (char*)lpBuff;
	const wchar_t* pWchar = (wchar_t*)lpBuff;
	int i = 0;

	for (; nSize > 0; nSize -= 16)
	{
		printf("%08X\t", sizeAddress);
		sizeAddress += 16;
		for (int i = 0; i < 16; i++)
		{
			ch = pAnsi[i];
			printf("%02X ", ch);
		}
		
		for (int i = 0; i < 16; i++)
		{
			printf("%c ", pAnsi[i] < 33 || pAnsi[i]>126 ? '.' : pAnsi[i]);
		}
		pAnsi += 16;
		printf("\n");
	}
}

void Memory::EditMem(HANDLE hProcess, SIZE_T sizeAddr, BYTE* pBuff, int nFlags)
{
	DWORD dwOldProtect = 0;
	DWORD dwBytes = 0;

	if (nFlags == 1)
	{
		int nLen = 1;
		// 修改内存的保护属性
		VirtualProtectEx(hProcess, (LPVOID)sizeAddr, nLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		// 在目标位置写入生成的 opcode 
		WriteMemory(hProcess, sizeAddr, pBuff, nLen);
		// 还原保护属性
		VirtualProtectEx(hProcess, (LPVOID)sizeAddr, nLen, dwOldProtect, &dwOldProtect);
	}
	else if (nFlags == 2)
	{
		int nLen = 2;
		// 修改内存的保护属性
		VirtualProtectEx(hProcess, (LPVOID)sizeAddr, nLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		// 在目标位置写入生成的 opcode 
		WriteMemory(hProcess, sizeAddr, pBuff, nLen);
		// 还原保护属性
		VirtualProtectEx(hProcess, (LPVOID)sizeAddr, nLen, dwOldProtect, &dwOldProtect);
	}
	else if (nFlags == 4)
	{
		int nLen = 4;
		// 修改内存的保护属性
		VirtualProtectEx(hProcess, (LPVOID)sizeAddr, nLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		// 在目标位置写入生成的 opcode 
		WriteMemory(hProcess, sizeAddr, pBuff, nLen);
		// 还原保护属性
		VirtualProtectEx(hProcess, (LPVOID)sizeAddr, nLen, dwOldProtect, &dwOldProtect);
	}
	else if (nFlags == 8)
	{
		int nLen = 8;
		// 修改内存的保护属性
		VirtualProtectEx(hProcess, (LPVOID)sizeAddr, nLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		// 在目标位置写入生成的 opcode 
		WriteMemory(hProcess, sizeAddr, pBuff, nLen);
		// 还原保护属性
		VirtualProtectEx(hProcess, (LPVOID)sizeAddr, nLen, dwOldProtect, &dwOldProtect);
	}
}