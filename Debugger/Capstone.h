#pragma once
#include <Windows.h>
#include "capstone\include\capstone.h"
#include "AssamblyEngine/XEDParse.h"
#pragma comment(lib, "capstone/lib/capstone_x86.lib")
#pragma comment(lib,"AssamblyEngine/XEDParse.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"libcmtd.lib\"")

typedef struct _XEDPARSE
{
	bool				x64;							// 等于 true 时，生成的opcode是64位的，等于false时，生成的opcode是32位的
	ULONGLONG			cip;							// 指令的当前地址
	unsigned			destSize;						// 生成的opcode的长度
	CBXEDPARSE_UNKNOWN	cbUnknown;						// 出现未定义的操作数时的回调函数
	unsigned char		dest[XEDPARSE_MAXASMSIZE];		// 生成的opcode
	char				instr[XEDPARSE_MAXASMSIZE];		// 需要转换成opcode的汇编指令
	char				error[XEDPARSE_MAXASMSIZE];		// 错误信息，函数返回false时，说明转换失败
};

class Capstone
{
private:
	// 用于初始化和内存管理的句柄
	static csh			m_hHandle;
	static cs_opt_mem	m_optMem;

public:
	Capstone() = default;
	~Capstone() = default;

	static	void Init();												// 初始化
	static	DWORD DisAsm(HANDLE hHandle, LPVOID pAddr, DWORD dwCount);	// 执行反汇编
	void	WriteDisAsm(HANDLE hProcess);								// 修改反汇编指令
};