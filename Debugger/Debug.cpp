#include "Debug.h"
#include "Capstone.h"
#include "BreakPoint.h"
#include "EFlags.h"
#include "PE.h"
#include "Stack.h"
#include "Module.h"
#include "Plugin.h"
#include "Symbol.h"
#include "PE.h"
#include "AntiDebug.h"

HANDLE g_hProcess;
HANDLE g_hThread;

void Debug::OpenFile(PCSTR lpFilePath)
{
	PROCESS_INFORMATION information = { 0 };
	STARTUPINFOA startupInfo = { sizeof(STARTUPINFOA) };

	m_lpFilePath = lpFilePath;

	// 以调试的方式打开目标进程
	BOOL success = CreateProcessA(lpFilePath, NULL, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
		NULL, NULL, &startupInfo, &information);

	g_hProcess = information.hProcess;
	g_hThread = information.hThread;

	// 关闭句柄
	if (success == TRUE)
	{
		CloseHandle(information.hThread);
		CloseHandle(information.hProcess);
	}

	// 初始化反汇编引擎
	Capstone::Init();
}

bool Debug::OpenPid(int exepid)
{
	Capstone::Init();
	return DebugActiveProcess(exepid);
}

void Debug::Run()
{
	// 运行插件
	Plugin::OnInit();
	
	// 循环的从调试对象的调试信息队列中获取到调试信息
	while (WaitForDebugEvent(&m_debugEvent, INFINITE))
	{
		// 每次获取到调试信息以后都打开句柄
		OpenHandles();

		switch (m_debugEvent.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:			// 异常调试事件
			OnExceptHandler();
			break;
		case CREATE_PROCESS_DEBUG_EVENT:	// 进程创建事件，获取OEP
			m_lpOep = m_debugEvent.u.CreateProcessInfo.lpStartAddress;
			m_processInfo = m_debugEvent.u.CreateProcessInfo;
			break;
		}

		// 谁触发了调试信息，就告诉系统是处理谁的调试信息
		ContinueDebugEvent(m_debugEvent.dwProcessId,
			m_debugEvent.dwThreadId, m_continueStatus);

		if (BreakPoint::m_breakList.size() != 0 &&
			BreakPoint::m_breakList.back().pAddress != m_lpOep)
		{
			BreakPoint::SetCCBreakpoint(m_hProcess, BreakPoint::m_breakList.back().pAddress);
			BreakPoint::m_breakList.erase(BreakPoint::m_breakList.end() - 1);
		}

		CloseHandles();		// 每次使用完都需要关闭句柄
	}
}

void Debug::OnExceptHandler()
{
	// 获取异常的类型和异常产生的地址
	DWORD dwCode = m_debugEvent.u.Exception.ExceptionRecord.ExceptionCode;
	PVOID pAddress = m_debugEvent.u.Exception.ExceptionRecord.ExceptionAddress;
	ULONG_PTR uInfo = m_debugEvent.u.Exception.ExceptionRecord.ExceptionInformation[0];
	LPVOID uAddress = (LPVOID)m_debugEvent.u.Exception.ExceptionRecord.ExceptionInformation[1];

	// 根据不同的异常类型进行相应的处理
	switch (dwCode)
	{
	case EXCEPTION_BREAKPOINT:			// 软件断点异常
	{
		if (m_boolIsSystemBreakpoint == true)
		{
			// 如果处于调试状态，就会通过int3 指令设置一个系统断点
			// 否则正常执行
			// 一般系统断点是不需要我们进程修复的
			m_boolIsSystemBreakpoint = false;
			BreakPoint::SetCCBreakpoint(m_hProcess, m_lpOep);
			m_boolNeedInput = false;

			// 获取模块
			Symbol::EnumProcessModule(m_debugEvent.dwProcessId);

			// 初始化符号
			Symbol::OnInitSym(g_hProcess, &m_processInfo);

			// 反反调试
			AntiDebug::HookNtQueryInformationProcess(m_hProcess);
		}

		// 如果产生了软件断点，就尝试修复
		m_continueStatus = BreakPoint::FixCCBreakpoint(m_hProcess, m_hThread, pAddress);

		break;
	}
	case EXCEPTION_SINGLE_STEP:
	{
		BreakPoint::FixHardBreakpoint(m_hThread, pAddress);
		
		// 保存用户设置的硬件断点
		m_continueStatus = DBG_CONTINUE;
		break;
	}
	case EXCEPTION_ACCESS_VIOLATION:
	{
		DWORD dwOld = 0;
		VirtualProtectEx(m_hProcess, uAddress, 1, PAGE_EXECUTE_READWRITE, &dwOld);
		break;
	}
	}

	if (m_boolNeedInput == true)
	{
		Capstone::DisAsm(m_hProcess, pAddress, 10);
		Input();
	}

	// 每次执行后恢复为需要输入
	m_boolNeedInput = true;
}

void Debug::Input()
{
	// 保存输入的命令
	char command[0x20] = { 0 };
	
	printf("kd> ");
	while (scanf_s("%s", command, 0x20) == 1)
	{
		if (!strcmp("g", command))			// 继续执行
		{
			break;
		}
		else if (!strcmp("u", command))			// 反汇编
		{
			LPVOID pAddress = 0;
			DWORD dwCount = 0;
			scanf_s("%X %d", &pAddress, &dwCount);
			Capstone::DisAsm(m_hProcess, pAddress, dwCount);
		}
		else if (!strcmp("eu", command))			// 修改反汇编指令
		{
			Capstone capstone;
			capstone.WriteDisAsm(m_hProcess);
		}
		else if (!strcmp("p", command))			// 执行 tf 断点，单步步入
		{
			BreakPoint::SetTFBreakpoint(m_hThread);
			break;
		}
		else if (!strcmp("t", command))			// 单步步过
		{
			SetBreakpoint();
			break;
		}
		else if (!strcmp("bp", command))			// 设置一个软件断点
		{
			LPVOID pAddress = 0;
			scanf_s("%X", &pAddress);
			BreakPoint::SetCCBreakpoint(m_hProcess, pAddress);
		}
		else if (!strcmp("bhe", command))		// 设置一个硬件断点
		{
			LPVOID pAddress = 0;
			scanf_s("%X", &pAddress);
			BreakPoint::SetHardBreakpoint(m_hThread, pAddress, 0, 0);
		}
		else if (!strcmp("bhr", command))		// 硬件读断点
		{
			ULONG_PTR uAddress = 0;
			scanf_s("%X", &uAddress);
			BreakPoint::SetBreakpointHardRw(m_hThread, uAddress, 1, 1);
		}
		else if (!strcmp("bhw", command))		// 硬件写断点
		{
			ULONG_PTR uAddress = 0;
			scanf_s("%X", &uAddress);
			BreakPoint::SetBreakpointHardRw(m_hThread, uAddress, 3, 3);
		}
		else if (!strcmp("r", command))			// 查看寄存器
		{
			EFlags::ShowRegisters(m_hThread);
		}
		else if (!strcmp("rb", command))			// 修改通用寄存器
		{
			char eFlags[10];
			DWORD dwNum = 0;
			scanf_s("%s %d", &eFlags, 10, &dwNum);
			EFlags::MemRegisters(m_hThread, eFlags, dwNum);
		}
		else if (!strcmp("rf", command))			// 修改标志寄存器
		{
			char eFlags[10];
			DWORD dwNum = 0;
			scanf_s("%s %d", &eFlags, 10, &dwNum);
			EFlags::MemEFlagsRegisters(m_hThread, eFlags, dwNum);
		}
		else if (!strcmp("k", command))			// 查看堆栈
		{
			Stack stack;
			stack.ShowStatic(m_hProcess, m_hThread);
		}
		else if (!strcmp("db", command))		// 查看内存
		{
			int nCount = 0;
			SIZE_T sizeAddress;
			BYTE lpBuff[16 * 10];
			scanf_s("%X", &sizeAddress);
			m_mMem.ReadMemory(m_hProcess, sizeAddress, lpBuff, 16 * 10);
			m_mMem.ShowMem(sizeAddress, lpBuff, 16 * 10);
		}
		else if (!strcmp("eb", command))		// 修改一个字节内存
		{
			SIZE_T sizeAddress;
			BYTE pBuff[20];
			scanf_s("%X %s", &sizeAddress, &pBuff, 20);
			m_mMem.EditMem(m_hProcess, sizeAddress, pBuff, nFlag);
		}
		else if (!strcmp("ew", command))		// 修改二个字节内存
		{
			nFlag = 2;
			SIZE_T sizeAddress;
			BYTE pBuff[20];
			scanf_s("%X %s", &sizeAddress, &pBuff, 20);
			m_mMem.EditMem(m_hProcess, sizeAddress, pBuff, nFlag);
		}
		else if (!strcmp("ed", command))		// 修改四个字节内存
		{
			nFlag = 4;
			SIZE_T sizeAddress;
			BYTE pBuff[20];
			scanf_s("%X %s", &sizeAddress, &pBuff, 20);
			m_mMem.EditMem(m_hProcess, sizeAddress, pBuff, nFlag);
		}
		else if (!strcmp("eq", command))		// 修改八个字节内存
		{
			nFlag = 8;
			SIZE_T sizeAddress;
			BYTE pBuff[20];
			scanf_s("%X %s", &sizeAddress, &pBuff, 20);
			m_mMem.EditMem(m_hProcess, sizeAddress, pBuff, nFlag);
		}
		else if (!strcmp("ee", command))		// 内存执行断点
		{
			DWORD dwZero = 0;
			LPVOID lpAddress;
			scanf_s("%X", &lpAddress);
			BreakPoint::SetMemBreakpoint(m_hProcess, lpAddress, 8, dwZero);
		}
		else if (!strcmp("ew", command))		// 内存写断点
		{
			DWORD dwZero = 0;
			LPVOID lpAddress;
			scanf_s("%X", &lpAddress);
			BreakPoint::SetMemBreakpoint(m_hProcess, lpAddress, 1, dwZero);
		}
		else if (!strcmp("er", command))		// 内存读断点
		{
			DWORD dwZero = 0;
			LPVOID lpAddress;
			scanf_s("%X", &lpAddress);
			BreakPoint::SetMemBreakpoint(m_hProcess, lpAddress, 0, dwZero);
		}
		else if (!strcmp("lm", command))		// 查看模块
		{
			Module::EnumModuleList(m_hProcess);
		}
		else if (!strcmp(".relname", command))	// 获取符号对应的地址
		{
			char szName[50]{};
			scanf_s("%s", &szName, _countof(szName));
			SIZE_T szAddress = Symbol::GetSymAddress(g_hProcess, szName);
			printf("%08X\n", szAddress);
		}
		else if (!strcmp(".reladdr", command))	// 获取地址对应的符号
		{
			CString str;
			SIZE_T szAddress;
			scanf_s("%X", &szAddress);
			Symbol::GetSymName(g_hProcess, szAddress, str);
			USES_CONVERSION;
			std::string s(W2A(str));
			const char* cstr = s.c_str();
			printf(cstr);
			printf("\n");
		}
		else if (!strcmp(".source", command))	// 获取指定某行的源码
		{
			SIZE_T szAddr;
			scanf_s("%X", &szAddr);
			Symbol::GetSource(g_hProcess, g_hThread, szAddr);
		}
		else if (!strcmp(".import", command))	// 获取导入表
		{
			char* pBuff = PE::OpenFile(m_lpFilePath);
			PE::EnumImport(pBuff);
		}
		else if (!strcmp(".export", command))	// 获取导出表
		{
			char* pBuff = PE::OpenFile(m_lpFilePath);
			PE::EnumExport(pBuff);
		}
		else if (!strcmp(".dump", command))	// dump
		{
			char pName[20] = { 0 };
			scanf_s("%s", &pName, 20);
			PE::Dump(pName, m_hProcess);
		}
		else if (!strcmp(".cls", command))		// 清屏
		{
			system("cls");
		}
		else if (!strcmp(".help", command))		// 帮助
		{
			printf("支持插件\n");
			printf("支持反反调试\n");
			printf(".help\t帮助\n");
			printf(".cls\t清屏\n");
			printf(".import\t获取导入表\n");
			printf(".export\t获取导出表\n");
			printf(".source\t获取指定某行的源码\n");
			printf(".relname\t<name>\t获取符号对应的地址\n");
			printf(".reladdr\t<address>\t获取地址对应的符号\n");
			printf(".dump\tdump\n");
			printf("g\t继续执行\n");
			printf("r\t查看寄存器\n");
			printf("p\t单步步入\n");
			printf("t\t单步步过\n");
			printf("k\t查看堆栈\n");
			printf("lm\t查看模块\n");
			printf("bp\t<address>(地址)\t软件断点\n");
			printf("bhe\t<address>(地址)\t硬件断点\n");
			printf("bhr\t<address>(地址)\t硬件读断点\n");
			printf("bhw\t<address>(地址)\t硬件写断点\n");
			printf("db\t<address>(地址)\t查看内存\n");
			printf("ee\t<address>(地址)\t内存执行断点\n");
			printf("ew\t<address>(地址)\t内存写断点\n");
			printf("er\t<address>(地址)\t内存访问断点\n");
			printf("rb\t<flags>(标志位大写)\t<hex>(新的地址)\t修改通用寄存器\n");
			printf("rf\t<flags>(标志位大写)\t<data>(新的数据)\t修改标志寄存器\n");
			printf("u\t<address>(地址)\t<count>(数量)\t显示反汇编\n");
			printf("eu\t<address>(地址)\t<opcode>(新的指令)\t修改反汇编\n");
			printf("eb\t<address>(地址)\t<data>(新的数据)\t修改一字节空间内存\n");
			printf("ew\t<address>(地址)\t<data>(新的数据)\t修改二字节空间内存\n");
			printf("ed\t<address>(地址)\t<data>(新的数据)\t修改四字节空间内存\n");
			printf("eq\t<address>(地址)\t<data>(新的数据)\t修改八字节空间内存\n");
		}
		else
		{
			printf("%s 不是合格的命令\n", command);
		}

		printf("kd> ");
	}
}

void Debug::OpenHandles()
{
	// 当一个进程以调试的方式打开另一个进程时，就可以获取到目标进程的所有权限
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_debugEvent.dwProcessId);
	m_hThread = OpenThread(PROCESS_ALL_ACCESS, FALSE, m_debugEvent.dwThreadId);
}

void Debug::CloseHandles()
{
	CloseHandle(m_hThread);
	CloseHandle(m_hProcess);
}

void Debug::SetBreakpoint()
{
	Memory mem;
	CONTEXT context = { CONTEXT_ALL };
	GetThreadContext(m_hThread, &context);
	SIZE_T uEip = context.Eip;
	BYTE byte[2] = { 0 };
	mem.ReadMemory(m_hProcess, uEip, byte, 2);
	char opcode[1] = { 0 };
	DWORD dwCodeLen = Capstone::DisAsm(m_hProcess, (LPVOID)uEip, 1);
	uEip += dwCodeLen;
	system("cls");
	if (byte[0] == 0xe8/*call*/
		|| byte[0] == 0xf3/*rep*/
		|| byte[0] == 0x9a/*call*/
		|| (byte[0] == 0xff && 0x10 <= byte[1] && byte[1] <= 0x1d)/*call*/
		)
	{
		BreakPoint::SetCCBreakpoint(m_hProcess, (LPVOID)uEip);
	}
	else
	{
		BreakPoint::SetTFBreakpoint(m_hThread);
	}
}
