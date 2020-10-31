#include "Module.h"

list<MODULELIST> Module::moduleList;

void Module::EnumModuleList(HANDLE hProcess)
{
	// ö�ٽ���ģ��
	DWORD  dwModule;
	HMODULE hModules[100] = {0};
	EnumProcessModulesEx(hProcess, hModules, 400, &dwModule, LIST_MODULES_ALL);
	MODULEINFO info = { 0 };
	char path[MAX_PATH];
	char* name;
	LONG32 addr;
	LONG32 size;
	moduleList.resize((dwModule-4)/4);	// ���ô�С
	list<MODULELIST>::iterator itr = moduleList.begin();
	printf("+------------------+----------+-------------+-----------------------------------+\n");
	printf("|     ���ػ�ַ     + ģ���С |   ģ����	|	ģ��·��					    |\n");
	printf("+------------------+----------+-------------+-----------------------------------+\n");
	// ѭ����ȡģ����Ϣ
	for (int i = 0; i < (dwModule - 4) / 4; i++)
	{
		// ��ȡģ��·��
		GetModuleFileNameExA(hProcess, hModules[i], path, MAX_PATH);
		
		// ��ȡģ��������Ϣ
		GetModuleInformation(hProcess, hModules[i], &info, sizeof(MODULEINFO));
		
		name = PathFindFileNameA(path);
		addr = (LONG32)info.lpBaseOfDll; // dll ��ַ
		size = info.SizeOfImage; // dll ��С
		++itr;
		printf("| %08X | %08X | %s | %s|\t\n", addr, size, name, path);
	}

	printf("+------------------+----------+----------------+-----------------------------------+\n");
}