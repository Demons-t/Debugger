#include "Module.h"

list<MODULELIST> Module::moduleList;

void Module::EnumModuleList(HANDLE hProcess)
{
	// 枚举进程模块
	DWORD  dwModule;
	HMODULE hModules[100] = {0};
	EnumProcessModulesEx(hProcess, hModules, 400, &dwModule, LIST_MODULES_ALL);
	MODULEINFO info = { 0 };
	char path[MAX_PATH];
	char* name;
	LONG32 addr;
	LONG32 size;
	moduleList.resize((dwModule-4)/4);	// 设置大小
	list<MODULELIST>::iterator itr = moduleList.begin();
	printf("+------------------+----------+-------------+-----------------------------------+\n");
	printf("|     加载基址     + 模块大小 |   模块名	|	模块路径					    |\n");
	printf("+------------------+----------+-------------+-----------------------------------+\n");
	// 循环获取模块信息
	for (int i = 0; i < (dwModule - 4) / 4; i++)
	{
		// 获取模块路径
		GetModuleFileNameExA(hProcess, hModules[i], path, MAX_PATH);
		
		// 获取模块其他信息
		GetModuleInformation(hProcess, hModules[i], &info, sizeof(MODULEINFO));
		
		name = PathFindFileNameA(path);
		addr = (LONG32)info.lpBaseOfDll; // dll 基址
		size = info.SizeOfImage; // dll 大小
		++itr;
		printf("| %08X | %08X | %s | %s|\t\n", addr, size, name, path);
	}

	printf("+------------------+----------+----------------+-----------------------------------+\n");
}