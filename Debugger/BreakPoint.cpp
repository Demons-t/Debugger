#include "BreakPoint.h"

vector<BREAKPOINT_INFO> BreakPoint::m_breakList;
DWORD	BreakPoint::m_dwFixCCindex = -1;				//修复的软件断点的下标
DWORD	BreakPoint::m_dwIsTypeBreak = 0;

void BreakPoint::SetTFBreakpoint(HANDLE hThread)
{
	// 初始化结构体
	CONTEXT context = { CONTEXT_CONTROL };

	// 获取到目标线程的线程环境
	GetThreadContext(hThread, &context);

	// 设置目标的 EFLAGS.TF 为 1
	context.EFlags |= 0x00000100;

	// 将设置后的内容应用到目标线程
	SetThreadContext(hThread, &context);
}

void BreakPoint::SetCCBreakpoint(HANDLE hProcess, LPVOID pAddress)
{
	// 创建结构体，初始化断点的信息
	BREAKPOINT_INFO info = { pAddress };

	// 保存需要设置断点的地址上的原始内容
	DWORD dwLength = 0;
	ReadProcessMemory(hProcess, pAddress, &info.byteOpcode, 1, &dwLength);

	// 修改目标进程指定地址的内容为 0xCC
	WriteProcessMemory(hProcess, pAddress, "\xCC", 1, &dwLength);

	// 将设置好的断点添加到断点列表
	m_breakList.push_back(info);
}

DWORD BreakPoint::FixCCBreakpoint(HANDLE hProcess, HANDLE hThread, LPVOID pAddrdss)
{
	// 遍历用户设置的所有断点
	for (int i = 0; i < m_breakList.size(); i++)
	{
		// 对比断点，找到对应的断点信息
		if (pAddrdss == m_breakList[i].pAddress)
		{
			// 重新写入设置断点之前的内存
			DWORD dwordLength = 0;
			WriteProcessMemory(hProcess, pAddrdss, &m_breakList[i].byteOpcode, 1, &dwordLength);

			// 获取目标线程的线程环境
			CONTEXT context = { CONTEXT_CONTROL };
			GetThreadContext(hThread, &context);

			// 由于是陷阱类异常，所以 eip-1
			context.Eip -= 1;

			// 将修改应用到目标线程
			SetThreadContext(hThread, &context);

			// 如果是永久断点，暂时设置为无效
			// 如果是临时，直接删除
			//m_breakList.erase(m_breakList.begin() + i);
			return DBG_CONTINUE;
		}
	}

	return DBG_EXCEPTION_NOT_HANDLED;
}

void BreakPoint::SetHardBreakpoint(HANDLE hThread, LPVOID pAddress, DWORD dwType, DWORD dwLen)
{
	// 获取目标线程的寄存器
	CONTEXT context = { CONTEXT_DEBUG_REGISTERS };
	GetThreadContext(hThread, &context);

	// 获取DR7 寄存器，得到硬件断点的开关状态
	PR7 dr7 = (PR7)&context.Dr7;

	// 判断没有使用的硬件断点，并设置
	if (dr7->L0 == 0)
	{
		// 如果 L0 是 0 就意味着没有使用这个硬件断点
		dr7->L0 = 1;

		// 设置断点的地址
		context.Dr0 = (DWORD)pAddress;

		// 设置断点的类型
		dr7->RW0 = dwType;

		// 设置断点的触发长度，如果是执行就需要是0
		dr7->LEN0 = dwLen;
	}
	else if (dr7->L1 == 0)
	{
		// 如果L1 是 0 就意味着没有使用这个硬件断点
		dr7->L1 = 1;

		// 设置断点的地址
		context.Dr1 = (DWORD)pAddress;

		// 设置断点的类型
		dr7->RW1 = dwType;

		// 设置断点的触发长度
		dr7->LEN1 = dwLen;
	}
	else if (dr7->L2 == 0)
	{
		// 如果L2 是 0 就意味着没有使用这个硬件断点
		dr7->L2 = 1;

		// 设置断点的地址
		context.Dr2 = (DWORD)pAddress;

		// 设置断点的类型
		dr7->RW2 = dwType;

		// 设置断点的触发长度
		dr7->LEN2 = dwLen;
	}
	else if (dr7->L3 == 0)
	{
		// 如果L3 是 0 就意味着没有使用这个硬件断点
		dr7->L3 = 1;

		// 设置断点的地址
		context.Dr3 = (DWORD)pAddress;

		// 设置断点的类型
		dr7->RW3 = dwType;

		// 设置断点的触发长度
		dr7->LEN3 = dwLen;
	}
	else
	{
		printf("没有可用的硬件断点\n");
	}

	// 将寄存器的修改应用到线程
	SetThreadContext(hThread, &context);
}

void BreakPoint::FixHardBreakpoint(HANDLE hThread, LPVOID pAddrdss)
{
	// 获取目标线程的寄存器
	CONTEXT context = { CONTEXT_DEBUG_REGISTERS };
	GetThreadContext(hThread, &context);

	// 获取 Dr7 寄存器，得到硬件断点的开关状态
	PR7 dr7 = (PR7)&context.Dr7;

	// 通过 Dr6 判断是谁被触发了
	switch (context.Dr6 & 0xf)
	{
	case 1:dr7->L0 = 0; break;
	case 2:dr7->L1 = 0; break;
	case 4:dr7->L2 = 0; break;
	case 8:dr7->L3 = 0; break;
	default:
		break;
	}

	// 将寄存器的修改应用到线程

	SetThreadContext(hThread, &context);
}

BOOL BreakPoint::SetBreakpointHardRw(HANDLE hThread, ULONG_PTR uAddress, DWORD dwType, DWORD dwLen)
{
	// 获取线程环境
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(hThread, &context);

	// 对地址和长度进行对齐处理
	if (dwLen == 1)		// 2 字节的对齐粒度
	{
		uAddress = uAddress - uAddress % 2;
	}
	else if (dwLen == 3)	// 4 字节对齐粒度
	{
		uAddress = uAddress - uAddress % 4;
	}
	else if (dwLen > 3)
	{
		return FALSE;
	}

	// 判断哪些寄存器没有使用
	R7* pDr7 = (R7*)&context.Dr7;
	if (pDr7->L0 == 0)			// DR0 没有被使用
	{
		// 必须将L0 置 1 ，才代表启用此硬件断点
		pDr7->L0 = 1;
		context.Dr0 = uAddress;
		pDr7->RW0 = dwType;
		pDr7->LEN0 = dwLen;
	}
	else if (pDr7->L1 == 0)		// DR1 没有被使用
	{
		pDr7->L1 = 1;
		context.Dr1 = uAddress;
		pDr7->RW1 = dwType;
		pDr7->LEN1 = dwLen;
	}
	else if (pDr7->L2 == 0)		// DR2 没有被使用
	{
		pDr7->L2 = 1;
		context.Dr2 = uAddress;
		pDr7->RW2 = dwType;
		pDr7->LEN2 = dwLen;
	}
	else if (pDr7->L3 == 0)		// DR3 没有被使用
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