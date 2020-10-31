#pragma once
#include <Windows.h>
#include "capstone\include\capstone.h"
#include "AssamblyEngine/XEDParse.h"
#pragma comment(lib, "capstone/lib/capstone_x86.lib")
#pragma comment(lib,"AssamblyEngine/XEDParse.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"libcmtd.lib\"")

typedef struct _XEDPARSE
{
	bool				x64;							// ���� true ʱ�����ɵ�opcode��64λ�ģ�����falseʱ�����ɵ�opcode��32λ��
	ULONGLONG			cip;							// ָ��ĵ�ǰ��ַ
	unsigned			destSize;						// ���ɵ�opcode�ĳ���
	CBXEDPARSE_UNKNOWN	cbUnknown;						// ����δ����Ĳ�����ʱ�Ļص�����
	unsigned char		dest[XEDPARSE_MAXASMSIZE];		// ���ɵ�opcode
	char				instr[XEDPARSE_MAXASMSIZE];		// ��Ҫת����opcode�Ļ��ָ��
	char				error[XEDPARSE_MAXASMSIZE];		// ������Ϣ����������falseʱ��˵��ת��ʧ��
};

class Capstone
{
private:
	// ���ڳ�ʼ�����ڴ����ľ��
	static csh			m_hHandle;
	static cs_opt_mem	m_optMem;

public:
	Capstone() = default;
	~Capstone() = default;

	static	void Init();												// ��ʼ��
	static	DWORD DisAsm(HANDLE hHandle, LPVOID pAddr, DWORD dwCount);	// ִ�з����
	void	WriteDisAsm(HANDLE hProcess);								// �޸ķ����ָ��
};