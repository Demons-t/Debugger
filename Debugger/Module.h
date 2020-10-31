#pragma once
#include <Windows.h>
#include <list>
#include <vector>
#include "psapi.h " 
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

using namespace std;

typedef struct _MODULELIST
{
	LPCSTR		lpName;
	LONG32		uStart;
	LONG32		uSize;
	LPVOID		lpFIlePath[MAX_PATH];
}MODULELIST, *PMODULELIST;

class Module
{
private:
	static list<MODULELIST> moduleList;

public:
	static void	EnumModuleList(HANDLE hProcess);			// Ã¶¾ÙÄ£¿é
};

