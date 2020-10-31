#include "BreakPoint.h"

vector<BREAKPOINT_INFO> BreakPoint::m_breakList;
DWORD	BreakPoint::m_dwFixCCindex = -1;				//�޸�������ϵ���±�
DWORD	BreakPoint::m_dwIsTypeBreak = 0;

void BreakPoint::SetTFBreakpoint(HANDLE hThread)
{
	// ��ʼ���ṹ��
	CONTEXT context = { CONTEXT_CONTROL };

	// ��ȡ��Ŀ���̵߳��̻߳���
	GetThreadContext(hThread, &context);

	// ����Ŀ��� EFLAGS.TF Ϊ 1
	context.EFlags |= 0x00000100;

	// �����ú������Ӧ�õ�Ŀ���߳�
	SetThreadContext(hThread, &context);
}

void BreakPoint::SetCCBreakpoint(HANDLE hProcess, LPVOID pAddress)
{
	// �����ṹ�壬��ʼ���ϵ����Ϣ
	BREAKPOINT_INFO info = { pAddress };

	// ������Ҫ���öϵ�ĵ�ַ�ϵ�ԭʼ����
	DWORD dwLength = 0;
	ReadProcessMemory(hProcess, pAddress, &info.byteOpcode, 1, &dwLength);

	// �޸�Ŀ�����ָ����ַ������Ϊ 0xCC
	WriteProcessMemory(hProcess, pAddress, "\xCC", 1, &dwLength);

	// �����úõĶϵ���ӵ��ϵ��б�
	m_breakList.push_back(info);
}

DWORD BreakPoint::FixCCBreakpoint(HANDLE hProcess, HANDLE hThread, LPVOID pAddrdss)
{
	// �����û����õ����жϵ�
	for (int i = 0; i < m_breakList.size(); i++)
	{
		// �Աȶϵ㣬�ҵ���Ӧ�Ķϵ���Ϣ
		if (pAddrdss == m_breakList[i].pAddress)
		{
			// ����д�����öϵ�֮ǰ���ڴ�
			DWORD dwordLength = 0;
			WriteProcessMemory(hProcess, pAddrdss, &m_breakList[i].byteOpcode, 1, &dwordLength);

			// ��ȡĿ���̵߳��̻߳���
			CONTEXT context = { CONTEXT_CONTROL };
			GetThreadContext(hThread, &context);

			// �������������쳣������ eip-1
			context.Eip -= 1;

			// ���޸�Ӧ�õ�Ŀ���߳�
			SetThreadContext(hThread, &context);

			// ��������öϵ㣬��ʱ����Ϊ��Ч
			// �������ʱ��ֱ��ɾ��
			//m_breakList.erase(m_breakList.begin() + i);
			return DBG_CONTINUE;
		}
	}

	return DBG_EXCEPTION_NOT_HANDLED;
}

void BreakPoint::SetHardBreakpoint(HANDLE hThread, LPVOID pAddress, DWORD dwType, DWORD dwLen)
{
	// ��ȡĿ���̵߳ļĴ���
	CONTEXT context = { CONTEXT_DEBUG_REGISTERS };
	GetThreadContext(hThread, &context);

	// ��ȡDR7 �Ĵ������õ�Ӳ���ϵ�Ŀ���״̬
	PR7 dr7 = (PR7)&context.Dr7;

	// �ж�û��ʹ�õ�Ӳ���ϵ㣬������
	if (dr7->L0 == 0)
	{
		// ��� L0 �� 0 ����ζ��û��ʹ�����Ӳ���ϵ�
		dr7->L0 = 1;

		// ���öϵ�ĵ�ַ
		context.Dr0 = (DWORD)pAddress;

		// ���öϵ������
		dr7->RW0 = dwType;

		// ���öϵ�Ĵ������ȣ������ִ�о���Ҫ��0
		dr7->LEN0 = dwLen;
	}
	else if (dr7->L1 == 0)
	{
		// ���L1 �� 0 ����ζ��û��ʹ�����Ӳ���ϵ�
		dr7->L1 = 1;

		// ���öϵ�ĵ�ַ
		context.Dr1 = (DWORD)pAddress;

		// ���öϵ������
		dr7->RW1 = dwType;

		// ���öϵ�Ĵ�������
		dr7->LEN1 = dwLen;
	}
	else if (dr7->L2 == 0)
	{
		// ���L2 �� 0 ����ζ��û��ʹ�����Ӳ���ϵ�
		dr7->L2 = 1;

		// ���öϵ�ĵ�ַ
		context.Dr2 = (DWORD)pAddress;

		// ���öϵ������
		dr7->RW2 = dwType;

		// ���öϵ�Ĵ�������
		dr7->LEN2 = dwLen;
	}
	else if (dr7->L3 == 0)
	{
		// ���L3 �� 0 ����ζ��û��ʹ�����Ӳ���ϵ�
		dr7->L3 = 1;

		// ���öϵ�ĵ�ַ
		context.Dr3 = (DWORD)pAddress;

		// ���öϵ������
		dr7->RW3 = dwType;

		// ���öϵ�Ĵ�������
		dr7->LEN3 = dwLen;
	}
	else
	{
		printf("û�п��õ�Ӳ���ϵ�\n");
	}

	// ���Ĵ������޸�Ӧ�õ��߳�
	SetThreadContext(hThread, &context);
}

void BreakPoint::FixHardBreakpoint(HANDLE hThread, LPVOID pAddrdss)
{
	// ��ȡĿ���̵߳ļĴ���
	CONTEXT context = { CONTEXT_DEBUG_REGISTERS };
	GetThreadContext(hThread, &context);

	// ��ȡ Dr7 �Ĵ������õ�Ӳ���ϵ�Ŀ���״̬
	PR7 dr7 = (PR7)&context.Dr7;

	// ͨ�� Dr6 �ж���˭��������
	switch (context.Dr6 & 0xf)
	{
	case 1:dr7->L0 = 0; break;
	case 2:dr7->L1 = 0; break;
	case 4:dr7->L2 = 0; break;
	case 8:dr7->L3 = 0; break;
	default:
		break;
	}

	// ���Ĵ������޸�Ӧ�õ��߳�

	SetThreadContext(hThread, &context);
}

BOOL BreakPoint::SetBreakpointHardRw(HANDLE hThread, ULONG_PTR uAddress, DWORD dwType, DWORD dwLen)
{
	// ��ȡ�̻߳���
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(hThread, &context);

	// �Ե�ַ�ͳ��Ƚ��ж��봦��
	if (dwLen == 1)		// 2 �ֽڵĶ�������
	{
		uAddress = uAddress - uAddress % 2;
	}
	else if (dwLen == 3)	// 4 �ֽڶ�������
	{
		uAddress = uAddress - uAddress % 4;
	}
	else if (dwLen > 3)
	{
		return FALSE;
	}

	// �ж���Щ�Ĵ���û��ʹ��
	R7* pDr7 = (R7*)&context.Dr7;
	if (pDr7->L0 == 0)			// DR0 û�б�ʹ��
	{
		// ���뽫L0 �� 1 ���Ŵ������ô�Ӳ���ϵ�
		pDr7->L0 = 1;
		context.Dr0 = uAddress;
		pDr7->RW0 = dwType;
		pDr7->LEN0 = dwLen;
	}
	else if (pDr7->L1 == 0)		// DR1 û�б�ʹ��
	{
		pDr7->L1 = 1;
		context.Dr1 = uAddress;
		pDr7->RW1 = dwType;
		pDr7->LEN1 = dwLen;
	}
	else if (pDr7->L2 == 0)		// DR2 û�б�ʹ��
	{
		pDr7->L2 = 1;
		context.Dr2 = uAddress;
		pDr7->RW2 = dwType;
		pDr7->LEN2 = dwLen;
	}
	else if (pDr7->L3 == 0)		// DR3 û�б�ʹ��
	{
		pDr7->L3 = 1;
		context.Dr3 = uAddress;
		pDr7->RW3 = dwType;
		pDr7->LEN3 = dwLen;
	}
	else
	{
		return FALSE;
	}

	SetThreadContext(hThread, &context);

	return TRUE;
}

void BreakPoint::SetMemBreakpoint(HANDLE hProcess, LPVOID pAddress, DWORD dwType, DWORD oldType)
{
	BREAKPOINT_INFO memInfo;
	memInfo.pAddress = pAddress;
	memInfo.dwType = dwType;

	if (dwType == 0)
	{
		VirtualProtectEx(hProcess, pAddress, 1, PAGE_NOACCESS, &memInfo.Zero);
	}
	else if (dwType == 1)
	{
		VirtualProtectEx(hProcess, pAddress, 1, PAGE_EXECUTE_READ, &memInfo.Zero);
	}
	else if (dwType == 8)
	{
		VirtualProtectEx(hProcess, pAddress, 1, PAGE_NOACCESS, &memInfo.Zero);
	}
	
	m_breakList.push_back(memInfo);
}