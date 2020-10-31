#include "AntiDebug.h"

void AntiDebug::HookNtQueryInformationProcess(HANDLE hProcess)
{
	struct PROCESS_BASIC_INFORMATION {
		ULONG ExitStatus;                   // 进程返回码
		INT  PebBaseAddress;				// PEB地址
		ULONG AffinityMask;                 // CPU亲和性掩码
		LONG  BasePriority;                 // 基本优先级
		ULONG UniqueProcessId;              // 本进程PID
		ULONG InheritedFromUniqueProcessId; // 父进程PID
	}stcProcInfo;

	// 查询到进程相关的基本信息，需要提供一个结构体进行接收
	NtQueryInformationProcess(
		hProcess,
		ProcessBasicInformation,
		&stcProcInfo,
		sizeof(stcProcInfo),
		NULL);

	// 通过 WriteProcessMemory 修改目标进程的 PEB
	DWORD len = 0;
	WriteProcessMemory(
		hProcess,
		(LPVOID)(stcProcInfo.PebBaseAddress + 2),
		"",
		1,
		&len);
}
