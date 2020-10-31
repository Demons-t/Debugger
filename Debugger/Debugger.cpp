#include "Debug.h"
#include <stdio.h>

//将程序的权限提升为 SeDebug 权限
bool EnableDebugPrivilege()
{
	HANDLE hToken = NULL;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;
	TOKEN_ELEVATION_TYPE ElevationType;
	DWORD dwSize = 0;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		MessageBoxA(NULL, "打开失败", 0, 0);
		return   FALSE;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
	{
		CloseHandle(hToken);
		MessageBoxA(NULL, "获取失败", 0, 0);
		return false;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; // 选择还是关闭
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		CloseHandle(hToken);
		MessageBoxA(NULL, "获取权限失败", 0, 0);
		return false;
	}
	if (GetTokenInformation(hToken, TokenElevationType, &ElevationType, sizeof(TOKEN_ELEVATION_TYPE), &dwSize))
	{
		// 如果令牌是以受限的权限运行 (TokenElevationTypeLimited) ,
		if (ElevationType == TokenElevationTypeLimited)
		{
			return	FALSE;
		}
	}
	return true;
}

int main()
{
	Debug debug;

	EnableDebugPrivilege();
	int nPid = 0;
	char lpFilePath[MAX_PATH] = "";
	while (true)
	{
		printf("请选择调试的方式：\n");
		printf("1. 请输入程序路径\t2. 请输入程序PID\n");
		int nNum = 0;
		scanf_s("%d", &nNum);
		if (nNum == 1)
		{
			printf("请输入程序路径：");
			scanf_s("%s", lpFilePath, MAX_PATH);
			debug.OpenFile((PCSTR)lpFilePath);
			system("cls");
			break;
		}
		else if (nNum == 2)
		{
			printf("请输入程序的PID：");
			scanf_s("%d", &nPid);
			bool boolRet = debug.OpenPid(nPid);
			system("cls");
			break;
		}
		else
		{
			printf("请输入正确的选项\n");
		}
	}

	debug.Run();

	return 0;
}