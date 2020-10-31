#pragma once
#include <Windows.h>
#include <winternl.h>
#pragma comment(lib, "ntdll.lib")

class AntiDebug
{
public:
	static void HookNtQueryInformationProcess(HANDLE hProcess);
};