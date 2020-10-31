#include "Symbol.h"
 std::vector<MODULEENTRY32> Symbol::vecModules;

//����ģ��
void Symbol::EnumProcessModule(DWORD PID)
{
	// 1. ��������
	HANDLE hSnap =
		CreateToolhelp32Snapshot(
			TH32CS_SNAPMODULE,/*��ʾ�������ֿ���*/
			PID /*Ҫ���ĸ�������ģ�����*/);
	MODULEENTRY32 mentry = { sizeof(MODULEENTRY32) };
	if (!Module32First(hSnap, &mentry)) {
		return;
	}
	do
	{
		vecModules.push_back(mentry);
		// 3. �õ���һ����Ϣ
	} while (Module32Next(hSnap, &mentry));
}

BOOL Symbol::OnInitSym(HANDLE hProcess, CREATE_PROCESS_DEBUG_INFO *info)
{
	if (SymInitialize(hProcess, NULL, FALSE))
	{
		for (int i = 0; i < vecModules.size(); ++i)
		{
			// ����ģ������ļ�
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

	// �������ֲ�ѯ������Ϣ�������pSymbol��
	if (!SymFromName(hProcess, pszName, pSymbol))
	{
		return 0;
	}

	return (SIZE_T)pSymbol->Address;	// ���غ�����ַ
}

SIZE_T Symbol::GetSymName(HANDLE hProcess, SIZE_T sizeAddress, CString & strName)
{
	DWORD64 dwDisplacement = 0;
	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;
	// ���ݵ�ַ��ȡ������Ϣ
	if (!SymFromAddr(hProcess, sizeAddress, &dwDisplacement, pSymbol))
	{
		return FALSE;
	}

	strName = pSymbol->Name;
	return TRUE;
}

void Symbol::GetSource(HANDLE hPrecess, HANDLE hThread, SIZE_T sizeAddress)
{
	// ��ȡEIP
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_ALL;

	// ��ȡ��Ŀ���̵߳��̻߳���
	GetThreadContext(hThread, &context);

	//��ȡԴ�ļ��Լ�����Ϣ
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
			// 126 ��ʾ��û��ͨ��SymLoadModule64����ģ����Ϣ
		case 126:
			printf("Debug info in current module has not loaded\n");
			return;

			// 487 ��ʾģ��û�е��Է���
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
		printf("�кţ�%d\n", lineInfo.LineNumber);
		printf("��ַ��%08X\n", lineInfo.Address);
		printf("�ļ�����%s\n", lineInfo.FileName);
	}
}