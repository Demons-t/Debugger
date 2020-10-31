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

	// �Ե��Եķ�ʽ��Ŀ�����
	BOOL success = CreateProcessA(lpFilePath, NULL, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
		NULL, NULL, &startupInfo, &information);

	g_hProcess = information.hProcess;
	g_hThread = information.hThread;

	// �رվ��
	if (success == TRUE)
	{
		CloseHandle(information.hThread);
		CloseHandle(information.hProcess);
	}

	// ��ʼ�����������
	Capstone::Init();
}

bool Debug::OpenPid(int exepid)
{
	Capstone::Init();
	return DebugActiveProcess(exepid);
}

void Debug::Run()
{
	// ���в��
	Plugin::OnInit();
	
	// ѭ���Ĵӵ��Զ���ĵ�����Ϣ�����л�ȡ��������Ϣ
	while (WaitForDebugEvent(&m_debugEvent, INFINITE))
	{
		// ÿ�λ�ȡ��������Ϣ�Ժ󶼴򿪾��
		OpenHandles();

		switch (m_debugEvent.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:			// �쳣�����¼�
			OnExceptHandler();
			break;
		case CREATE_PROCESS_DEBUG_EVENT:	// ���̴����¼�����ȡOEP
			m_lpOep = m_debugEvent.u.CreateProcessInfo.lpStartAddress;
			m_processInfo = m_debugEvent.u.CreateProcessInfo;
			break;
		}

		// ˭�����˵�����Ϣ���͸���ϵͳ�Ǵ���˭�ĵ�����Ϣ
		ContinueDebugEvent(m_debugEvent.dwProcessId,
			m_debugEvent.dwThreadId, m_continueStatus);

		if (BreakPoint::m_breakList.size() != 0 &&
			BreakPoint::m_breakList.back().pAddress != m_lpOep)
		{
			BreakPoint::SetCCBreakpoint(m_hProcess, BreakPoint::m_breakList.back().pAddress);
			BreakPoint::m_breakList.erase(BreakPoint::m_breakList.end() - 1);
		}

		CloseHandles();		// ÿ��ʹ���궼��Ҫ�رվ��
	}
}

void Debug::OnExceptHandler()
{
	// ��ȡ�쳣�����ͺ��쳣�����ĵ�ַ
	DWORD dwCode = m_debugEvent.u.Exception.ExceptionRecord.ExceptionCode;
	PVOID pAddress = m_debugEvent.u.Exception.ExceptionRecord.ExceptionAddress;
	ULONG_PTR uInfo = m_debugEvent.u.Exception.ExceptionRecord.ExceptionInformation[0];
	LPVOID uAddress = (LPVOID)m_debugEvent.u.Exception.ExceptionRecord.ExceptionInformation[1];

	// ���ݲ�ͬ���쳣���ͽ�����Ӧ�Ĵ���
	switch (dwCode)
	{
	case EXCEPTION_BREAKPOINT:			// ����ϵ��쳣
	{
		if (m_boolIsSystemBreakpoint == true)
		{
			// ������ڵ���״̬���ͻ�ͨ��int3 ָ������һ��ϵͳ�ϵ�
			// ��������ִ��
			// һ��ϵͳ�ϵ��ǲ���Ҫ���ǽ����޸���
			m_boolIsSystemBreakpoint = false;
			BreakPoint::SetCCBreakpoint(m_hProcess, m_lpOep);
			m_boolNeedInput = false;

			// ��ȡģ��
			Symbol::EnumProcessModule(m_debugEvent.dwProcessId);

			// ��ʼ������
			Symbol::OnInitSym(g_hProcess, &m_processInfo);

			// ��������
			AntiDebug::HookNtQueryInformationProcess(m_hProcess);
		}

		// �������������ϵ㣬�ͳ����޸�
		m_continueStatus = BreakPoint::FixCCBreakpoint(m_hProcess, m_hThread, pAddress);

		break;
	}
	case EXCEPTION_SINGLE_STEP:
	{
		BreakPoint::FixHardBreakpoint(m_hThread, pAddress);
		
		// �����û����õ�Ӳ���ϵ�
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

	// ÿ��ִ�к�ָ�Ϊ��Ҫ����
	m_boolNeedInput = true;
}

void Debug::Input()
{
	// �������������
	char command[0x20] = { 0 };
	
	printf("kd> ");
	while (scanf_s("%s", command, 0x20) == 1)
	{
		if (!strcmp("g", command))			// ����ִ��
		{
			break;
		}
		else if (!strcmp("u", command))			// �����
		{
			LPVOID pAddress = 0;
			DWORD dwCount = 0;
			scanf_s("%X %d", &pAddress, &dwCount);
			Capstone::DisAsm(m_hProcess, pAddress, dwCount);
		}
		else if (!strcmp("eu", command))			// �޸ķ����ָ��
		{
			Capstone capstone;
			capstone.WriteDisAsm(m_hProcess);
		}
		else if (!strcmp("p", command))			// ִ�� tf �ϵ㣬��������
		{
			BreakPoint::SetTFBreakpoint(m_hThread);
			break;
		}
		else if (!strcmp("t", command))			// ��������
		{
			SetBreakpoint();
			break;
		}
		else if (!strcmp("bp", command))			// ����һ������ϵ�
		{
			LPVOID pAddress = 0;
			scanf_s("%X", &pAddress);
			BreakPoint::SetCCBreakpoint(m_hProcess, pAddress);
		}
		else if (!strcmp("bhe", command))		// ����һ��Ӳ���ϵ�
		{
			LPVOID pAddress = 0;
			scanf_s("%X", &pAddress);
			BreakPoint::SetHardBreakpoint(m_hThread, pAddress, 0, 0);
		}
		else if (!strcmp("bhr", command))		// Ӳ�����ϵ�
		{
			ULONG_PTR uAddress = 0;
			scanf_s("%X", &uAddress);
			BreakPoint::SetBreakpointHardRw(m_hThread, uAddress, 1, 1);
		}
		else if (!strcmp("bhw", command))		// Ӳ��д�ϵ�
		{
			ULONG_PTR uAddress = 0;
			scanf_s("%X", &uAddress);
			BreakPoint::SetBreakpointHardRw(m_hThread, uAddress, 3, 3);
		}
		else if (!strcmp("r", command))			// �鿴�Ĵ���
		{
			EFlags::ShowRegisters(m_hThread);
		}
		else if (!strcmp("rb", command))			// �޸�ͨ�üĴ���
		{
			char eFlags[10];
			DWORD dwNum = 0;
			scanf_s("%s %d", &eFlags, 10, &dwNum);
			EFlags::MemRegisters(m_hThread, eFlags, dwNum);
		}
		else if (!strcmp("rf", command))			// �޸ı�־�Ĵ���
		{
			char eFlags[10];
			DWORD dwNum = 0;
			scanf_s("%s %d", &eFlags, 10, &dwNum);
			EFlags::MemEFlagsRegisters(m_hThread, eFlags, dwNum);
		}
		else if (!strcmp("k", command))			// �鿴��ջ
		{
			Stack stack;
			stack.ShowStatic(m_hProcess, m_hThread);
		}
		else if (!strcmp("db", command))		// �鿴�ڴ�
		{
			int nCount = 0;
			SIZE_T sizeAddress;
			BYTE lpBuff[16 * 10];
			scanf_s("%X", &sizeAddress);
			m_mMem.ReadMemory(m_hProcess, sizeAddress, lpBuff, 16 * 10);
			m_mMem.ShowMem(sizeAddress, lpBuff, 16 * 10);
		}
		else if (!strcmp("eb", command))		// �޸�һ���ֽ��ڴ�
		{
			SIZE_T sizeAddress;
			BYTE pBuff[20];
			scanf_s("%X %s", &sizeAddress, &pBuff, 20);
			m_mMem.EditMem(m_hProcess, sizeAddress, pBuff, nFlag);
		}
		else if (!strcmp("ew", command))		// �޸Ķ����ֽ��ڴ�
		{
			nFlag = 2;
			SIZE_T sizeAddress;
			BYTE pBuff[20];
			scanf_s("%X %s", &sizeAddress, &pBuff, 20);
			m_mMem.EditMem(m_hProcess, sizeAddress, pBuff, nFlag);
		}
		else if (!strcmp("ed", command))		// �޸��ĸ��ֽ��ڴ�
		{
			nFlag = 4;
			SIZE_T sizeAddress;
			BYTE pBuff[20];
			scanf_s("%X %s", &sizeAddress, &pBuff, 20);
			m_mMem.EditMem(m_hProcess, sizeAddress, pBuff, nFlag);
		}
		else if (!strcmp("eq", command))		// �޸İ˸��ֽ��ڴ�
		{
			nFlag = 8;
			SIZE_T sizeAddress;
			BYTE pBuff[20];
			scanf_s("%X %s", &sizeAddress, &pBuff, 20);
			m_mMem.EditMem(m_hProcess, sizeAddress, pBuff, nFlag);
		}
		else if (!strcmp("ee", command))		// �ڴ�ִ�жϵ�
		{
			DWORD dwZero = 0;
			LPVOID lpAddress;
			scanf_s("%X", &lpAddress);
			BreakPoint::SetMemBreakpoint(m_hProcess, lpAddress, 8, dwZero);
		}
		else if (!strcmp("ew", command))		// �ڴ�д�ϵ�
		{
			DWORD dwZero = 0;
			LPVOID lpAddress;
			scanf_s("%X", &lpAddress);
			BreakPoint::SetMemBreakpoint(m_hProcess, lpAddress, 1, dwZero);
		}
		else if (!strcmp("er", command))		// �ڴ���ϵ�
		{
			DWORD dwZero = 0;
			LPVOID lpAddress;
			scanf_s("%X", &lpAddress);
			BreakPoint::SetMemBreakpoint(m_hProcess, lpAddress, 0, dwZero);
		}
		else if (!strcmp("lm", command))		// �鿴ģ��
		{
			Module::EnumModuleList(m_hProcess);
		}
		else if (!strcmp(".relname", command))	// ��ȡ���Ŷ�Ӧ�ĵ�ַ
		{
			char szName[50]{};
			scanf_s("%s", &szName, _countof(szName));
			SIZE_T szAddress = Symbol::GetSymAddress(g_hProcess, szName);
			printf("%08X\n", szAddress);
		}
		else if (!strcmp(".reladdr", command))	// ��ȡ��ַ��Ӧ�ķ���
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
		else if (!strcmp(".source", command))	// ��ȡָ��ĳ�е�Դ��
		{
			SIZE_T szAddr;
			scanf_s("%X", &szAddr);
			Symbol::GetSource(g_hProcess, g_hThread, szAddr);
		}
		else if (!strcmp(".import", command))	// ��ȡ�����
		{
			char* pBuff = PE::OpenFile(m_lpFilePath);
			PE::EnumImport(pBuff);
		}
		else if (!strcmp(".export", command))	// ��ȡ������
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
		else if (!strcmp(".cls", command))		// ����
		{
			system("cls");
		}
		else if (!strcmp(".help", command))		// ����
		{
			printf("֧�ֲ��\n");
			printf("֧�ַ�������\n");
			printf(".help\t����\n");
			printf(".cls\t����\n");
			printf(".import\t��ȡ�����\n");
			printf(".export\t��ȡ������\n");
			printf(".source\t��ȡָ��ĳ�е�Դ��\n");
			printf(".relname\t<name>\t��ȡ���Ŷ�Ӧ�ĵ�ַ\n");
			printf(".reladdr\t<address>\t��ȡ��ַ��Ӧ�ķ���\n");
			printf(".dump\tdump\n");
			printf("g\t����ִ��\n");
			printf("r\t�鿴�Ĵ���\n");
			printf("p\t��������\n");
			printf("t\t��������\n");
			printf("k\t�鿴��ջ\n");
			printf("lm\t�鿴ģ��\n");
			printf("bp\t<address>(��ַ)\t����ϵ�\n");
			printf("bhe\t<address>(��ַ)\tӲ���ϵ�\n");
			printf("bhr\t<address>(��ַ)\tӲ�����ϵ�\n");
			printf("bhw\t<address>(��ַ)\tӲ��д�ϵ�\n");
			printf("db\t<address>(��ַ)\t�鿴�ڴ�\n");
			printf("ee\t<address>(��ַ)\t�ڴ�ִ�жϵ�\n");
			printf("ew\t<address>(��ַ)\t�ڴ�д�ϵ�\n");
			printf("er\t<address>(��ַ)\t�ڴ���ʶϵ�\n");
			printf("rb\t<flags>(��־λ��д)\t<hex>(�µĵ�ַ)\t�޸�ͨ�üĴ���\n");
			printf("rf\t<flags>(��־λ��д)\t<data>(�µ�����)\t�޸ı�־�Ĵ���\n");
			printf("u\t<address>(��ַ)\t<count>(����)\t��ʾ�����\n");
			printf("eu\t<address>(��ַ)\t<opcode>(�µ�ָ��)\t�޸ķ����\n");
			printf("eb\t<address>(��ַ)\t<data>(�µ�����)\t�޸�һ�ֽڿռ��ڴ�\n");
			printf("ew\t<address>(��ַ)\t<data>(�µ�����)\t�޸Ķ��ֽڿռ��ڴ�\n");
			printf("ed\t<address>(��ַ)\t<data>(�µ�����)\t�޸����ֽڿռ��ڴ�\n");
			printf("eq\t<address>(��ַ)\t<data>(�µ�����)\t�޸İ��ֽڿռ��ڴ�\n");
		}
		else
		{
			printf("%s ���Ǻϸ������\n", command);
		}

		printf("kd> ");
	}
}

void Debug::OpenHandles()
{
	// ��һ�������Ե��Եķ�ʽ����һ������ʱ���Ϳ��Ի�ȡ��Ŀ����̵�����Ȩ��
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
