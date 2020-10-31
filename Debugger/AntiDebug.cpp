#include "AntiDebug.h"

void AntiDebug::HookNtQueryInformationProcess(HANDLE hProcess)
{
	struct PROCESS_BASIC_INFORMATION {
		ULONG ExitStatus;                   // ���̷�����
		INT  PebBaseAddress;				// PEB��ַ
		ULONG AffinityMask;                 // CPU�׺�������
		LONG  BasePriority;                 // �������ȼ�
		ULONG UniqueProcessId;              // ������PID
		ULONG InheritedFromUniqueProcessId; // ������PID
	}stcProcInfo;

	// ��ѯ��������صĻ�����Ϣ����Ҫ�ṩһ���ṹ����н���
	NtQueryInformationProcess(
		hProcess,
		ProcessBasicInformation,
		&stcProcInfo,
		sizeof(stcProcInfo),
		NULL);

	// ͨ�� WriteProcessMemory �޸�Ŀ����̵� PEB
	DWORD len = 0;
	WriteProcessMemory(
		hProcess,
		(LPVOID)(stcProcInfo.PebBaseAddress + 2),
		"",
		1,
		&len);
}
