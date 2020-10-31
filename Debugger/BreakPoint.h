#pragma once
#include <Windows.h>
#include <vector>

using namespace std;

typedef struct _BREAKPOINT_INFO
{
	LPVOID	pAddress;
	BYTE	byteOpcode;
	DWORD	dwType;
	DWORD	Zero;
}BREAKPOINT_INFO, *PBREAKPOINT_INFO;

// DR7 �Ĵ����ṹ��
typedef struct _DBG_PEG7
{
	unsigned L0 : 1; unsigned G0 : 1;
	unsigned L1 : 1; unsigned G1 : 1;
	unsigned L2 : 1; unsigned G2 : 1;
	unsigned L3 : 1; unsigned G3 : 1;
	unsigned LE : 1; unsigned GE : 1;
	unsigned : 6;							// ��������Ч�ռ�
	unsigned RW0 : 2; unsigned LEN0 : 2;
	unsigned RW1 : 2; unsigned LEN1 : 2;
	unsigned RW2 : 2; unsigned LEN2 : 2;
	unsigned RW3 : 2; unsigned LEN3 : 2;
}R7, *PR7;

typedef enum
{
	breakpointTypeE,	// �ڴ�ִ�жϵ�
	breakpointTypeW,	// �ڴ�д��ϵ�
	breakpointTypeF		// �ڴ���ʶϵ�
}BPTYPE;

class BreakPoint
{
public:
	static	DWORD		m_dwIsTypeBreak;
	static	vector		<BREAKPOINT_INFO> m_breakList;															// �������жϵ�
	static	DWORD		m_dwFixCCindex;																			// �޸�������ϵ���±�
	static	void		SetTFBreakpoint(HANDLE hThread);														// ���õ�������
	static	void		SetCCBreakpoint(HANDLE hProcess, LPVOID pAddress);										// ����һ������ϵ�
	static	DWORD		FixCCBreakpoint(HANDLE hProcess, HANDLE hThread, LPVOID pAddrdss);						// �޸�һ������ϵ�
	static	void		SetHardBreakpoint(HANDLE hThread, LPVOID pAddress, DWORD dwType, DWORD dwLen);			// ����Ӳ���ϵ�
	static	void		FixHardBreakpoint(HANDLE hThread, LPVOID pAddrdss);										// �޸�һ��Ӳ���ϵ�
	static	BOOL		SetBreakpointHardRw(HANDLE hThread, ULONG_PTR uAddress, DWORD dwType, DWORD dwLen);		// ����Ӳ����д�ϵ�
	static  void		SetMemBreakpoint(HANDLE hProcess, LPVOID pAddress, DWORD dwType, DWORD oldType);		// �����ڴ���ʶϵ�
};

