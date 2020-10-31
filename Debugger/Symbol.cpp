#include "Symbol.h"
 std::vector<MODULEENTRY32> Symbol::vecModules;

//遍历模块
void Symbol::EnumProcessModule(DWORD PID)
{
	// 1. 创建快照
	HANDLE hSnap =
		CreateToolhelp32Snapshot(
			TH32CS_SNAPMODULE,/*表示创建何种快照*/
			PID /*要给哪个进程拍模块快照*/);
	MODULEENTRY32 mentry = { sizeof(MODULEENTRY32) };
	if (!Module32First(hSnap, &mentry)) {
		return;
	}
	do
	{
		vecModules.push_back(mentry);
		// 3. 得到下一个信息
	} while (Module32Next(hSnap, &mentry));
}

BOOL Symbol::OnInitSym(HANDLE hProcess, CREATE_PROCESS_DEBUG_INFO *info)
{
	if (SymInitialize(hProcess, NULL, FALSE))
	{
		for (int i = 0; i < vecModules.size(); ++i)
		{
			// 加载模块符号文件
			char path[MAX_PATH] = {  };
			WideCharToMultiByte(CP_ACP, NULL, vecModules[i].szExePath, -1, path, _countof(path), NULL, FALSE);
			DWORD64 dwModule = SymLoadModuleEx(
				hProcess,
				NULL,
				path,
				NULL,
				0,
				0,
				0,
				0);
			if (dwModule == 0)
			{
				return FALSE;
			}
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

SIZE_T Symbol::GetSymAddress(HANDLE hProcess, const char * pszName)
{
	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];

	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	// 根据名字查询符号信息，输出到pSymbol中
	if (!SymFromName(hProcess, pszName, pSymbol))
	{
		return 0;
	}

	return (SIZE_T)pSymbol->Address;	// 返回函数地址
}

SIZE_T Symbol::GetSymName(HANDLE hProcess, SIZE_T sizeAddress, CString & strName)
{
	DWORD64 dwDisplacement = 0;
	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;
	// 根据地址获取符号信息
	if (!SymFromAddr(hProcess, sizeAddress, &dwDisplacement, pSymbol))
	{
		return FALSE;
	}

	strName = pSymbol->Name;
	return TRUE;
}

void Symbol::GetSource(HANDLE hPrecess, HANDLE hThread, SIZE_T sizeAddress)
{
	// 获取EIP
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_ALL;

	// 获取到目标线程的线程环境
	GetThreadContext(hThread, &context);

	//获取源文件以及行信息
	IMAGEHLP_LINE64 lineInfo = { 0 };
	SymSetOptions(SYMOPT_LOAD_LINES);
	lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	DWORD displacement = 0;

	if (SymGetLineFromAddr64(
		hPrecess,
		sizeAddress,
		&displacement,
		&lineInfo) == FALSE)
	{
		DWORD errorCode = GetLastError();

		switch (errorCode)
		{
			// 126 表示还没有通过SymLoadModule64加载模块信息
		case 126:
			printf("Debug info in current module has not loaded\n");
			return;

			// 487 表示模块没有调试符号
		case 487:
			printf("No debug info in current module\n");
			return;

		default:
			printf("SymGetLineFromAddr64 failed: %s\n", errorCode);
			return;
		}
	}
	else
	{
		printf("行号：%d\n", lineInfo.LineNumber);
		printf("地址：%08X\n", lineInfo.Address);
		printf("文件名：%s\n", lineInfo.FileName);
	}
}