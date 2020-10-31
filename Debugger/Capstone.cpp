#include "Capstone.h"

csh Capstone::m_hHandle = { 0 };
cs_opt_mem Capstone::m_optMem = { 0 };

void Capstone::Init()
{
	cs_err err;

	// ���öѿռ�Ļص�����
	m_optMem.free = free;
	m_optMem.calloc = calloc;
	m_optMem.malloc = malloc;
	m_optMem.realloc = realloc;
	m_optMem.vsnprintf = (cs_vsnprintf_t)vsprintf_s;

	// ע��ѿռ�����麯��
	cs_option(NULL, CS_OPT_MEM, (size_t)&m_optMem);

	// �򿪾��
	err = cs_open(CS_ARCH_X86, CS_MODE_32, &m_hHandle);

	if (err != CS_ERR_OK)
	{
		return;
	}
}

DWORD Capstone::DisAsm(HANDLE hHandle, LPVOID pAddr, DWORD dwCount)
{
	DWORD ret = 0;
	// ��ȡָ��λ���ڴ�Ļ�������Ϣ
	cs_insn* pInsn = nullptr;
	PCHAR buff = new CHAR[dwCount * 16]{ 0 };

	// ��ȡָ�����ȵ��ڴ�ռ�
	DWORD dwWrite = 0;
	ReadProcessMemory(hHandle, (LPVOID)pAddr, buff, dwCount * 16, &dwWrite);
	int nCount = cs_disasm(m_hHandle, (uint8_t*)buff, dwCount * 16, (uint64_t)pAddr, 0, &pInsn);

	for (DWORD i = 0; i < dwCount; i++)
	{
		printf("0x%08X\t", (UINT)pInsn[i].address);
		
		for (uint16_t it = 0; it < 16; it++)
		{
			if (it < pInsn[i].size)
			{
				ret = pInsn[i].size;
				printf("%02X ", pInsn[i].bytes[it]);
			}
			else
				printf("  ");
		}

		// ��������
		printf("\t%s %s\n", pInsn[i].mnemonic, pInsn[i].op_str);
	}

	printf("\n");

	// �ͷſռ�
	delete[] buff;
	cs_free(pInsn, nCount);
	return ret;
}

void Capstone::WriteDisAsm(HANDLE hProcess)
{
	// ��ʼ��һ���ṹ��
	XEDPARSE xed = { 0 };

	// ��������opcode�ĳ�ʼ��ַ
	scanf_s("%X", &xed.cip);

	// ʹ�� gets_s() ������������
	gets_s(xed.instr, XEDPARSE_MAXBUFSIZE);

	// ʹ�� XEDParseAssemnle() ���������ָ��ת���� opcode
	if (XEDPARSE_OK != XEDParseAssemble(&xed))
	{
		printf("ָ�����%s\n", xed.error);
	}

	// д�뵽�ڴ�
	DWORD dwOldProtect = 0;
	DWORD dwBytes = 0;

	// �޸��ڴ�ı�������
	VirtualProtectEx(hProcess, (LPVOID)xed.cip, xed.dest_size, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	// ��Ŀ��λ��д�����ɵ� opcode 
	WriteProcessMemory(hProcess, (LPVOID)xed.cip, xed.dest, xed.dest_size, &dwBytes);

	// ��ԭ��������
	VirtualProtectEx(hProcess, (LPVOID)xed.cip, xed.dest_size, dwOldProtect, &dwOldProtect);
}