#include "Capstone.h"

csh Capstone::m_hHandle = { 0 };
cs_opt_mem Capstone::m_optMem = { 0 };

void Capstone::Init()
{
	cs_err err;

	// 配置堆空间的回调函数
	m_optMem.free = free;
	m_optMem.calloc = calloc;
	m_optMem.malloc = malloc;
	m_optMem.realloc = realloc;
	m_optMem.vsnprintf = (cs_vsnprintf_t)vsprintf_s;

	// 注册堆空间管理组函数
	cs_option(NULL, CS_OPT_MEM, (size_t)&m_optMem);

	// 打开句柄
	err = cs_open(CS_ARCH_X86, CS_MODE_32, &m_hHandle);

	if (err != CS_ERR_OK)
	{
		return;
	}
}

DWORD Capstone::DisAsm(HANDLE hHandle, LPVOID pAddr, DWORD dwCount)
{
	DWORD ret = 0;
	// 读取指令位置内存的缓冲区信息
	cs_insn* pInsn = nullptr;
	PCHAR buff = new CHAR[dwCount * 16]{ 0 };

	// 读取指定长度的内存空间
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

		// 输出反汇编
		printf("\t%s %s\n", pInsn[i].mnemonic, pInsn[i].op_str);
	}

	printf("\n");

	// 释放空间
	delete[] buff;
	cs_free(pInsn, nCount);
	return ret;
}

void Capstone::WriteDisAsm(HANDLE hProcess)
{
	// 初始化一个结构体
	XEDPARSE xed = { 0 };

	// 接收生成opcode的初始地址
	scanf_s("%X", &xed.cip);

	// 使用 gets_s() 函数接收输入
	gets_s(xed.instr, XEDPARSE_MAXBUFSIZE);

	// 使用 XEDParseAssemnle() 函数将汇编指令转换成 opcode
	if (XEDPARSE_OK != XEDParseAssemble(&xed))
	{
		printf("指令错误：%s\n", xed.error);
	}

	// 写入到内存
	DWORD dwOldProtect = 0;
	DWORD dwBytes = 0;

	// 修改内存的保护属性
	VirtualProtectEx(hProcess, (LPVOID)xed.cip, xed.dest_size, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	// 在目标位置写入生成的 opcode 
	WriteProcessMemory(hProcess, (LPVOID)xed.cip, xed.dest, xed.dest_size, &dwBytes);

	// 还原保护属性
	VirtualProtectEx(hProcess, (LPVOID)xed.cip, xed.dest_size, dwOldProtect, &dwOldProtect);
}