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

// DR7 寄存器结构体
typedef struct _DBG_PEG7
{
	unsigned L0 : 1; unsigned G0 : 1;
	unsigned L1 : 1; unsigned G1 : 1;
	unsigned L2 : 1; unsigned G2 : 1;
	unsigned L3 : 1; unsigned G3 : 1;
	unsigned LE : 1; unsigned GE : 1;
	unsigned : 6;							// 保留的无效空间
	unsigned RW0 : 2; unsigned LEN0 : 2;
	unsigned RW1 : 2; unsigned LEN1 : 2;
	unsigned RW2 : 2; unsigned LEN2 : 2;
	unsigned RW3 : 2; unsigned LEN3 : 2;
}R7, *PR7;

typedef enum
{
	breakpointTypeE,	// 内存执行断点
	breakpointTypeW,	// 内存写入断点
	breakpointTypeF		// 内存访问断点
}BPTYPE;

class BreakPoint
{
public:
	static	DWORD		m_dwIsTypeBreak;
	static	vector		<BREAKPOINT_INFO> m_breakList;															// 保存所有断点
	static	DWORD		m_dwFixCCindex;																			// 修复的软件断点的下标
	static	void		SetTFBreakpoint(HANDLE hThread);														// 设置单步步入
	static	void		SetCCBreakpoint(HANDLE hProcess, LPVOID pAddress);										// 设置一个软件断点
	static	DWORD		FixCCBreakpoint(HANDLE hProcess, HANDLE hThread, LPVOID pAddrdss);						// 修复一个软件断点
	static	void		SetHardBreakpoint(HANDLE hThread, LPVOID pAddress, DWORD dwType, DWORD dwLen);			// 设置硬件断点
	static	void		FixHardBreakpoint(HANDLE hThread, LPVOID pAddrdss);										// 修复一个硬件断点
	static	BOOL		SetBreakpointHardRw(HANDLE hThread, ULONG_PTR uAddress, DWORD dwType, DWORD dwLen);		// 设置硬件读写断点
	static  void		SetMemBreakpoint(HANDLE hProcess, LPVOID pAddress, DWORD dwType, DWORD oldType);		// 设置内存访问断点
};

