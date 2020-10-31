#pragma once
#include <Windows.h>

// 标志寄存器位段信息结构体
typedef struct _EFLAGS
{
	unsigned CF			: 1;		// 进位或错位
	unsigned Reservel	: 1;
	unsigned PF			: 1;		// 计算结果低位包含偶数个数 1 时，此标志为 1
	unsigned Reserve2	: 2;
	unsigned AF			: 1;		// 辅助进位标志，当位 3 处有进位或借位时该标志位1
	unsigned Reserve3	: 3;
	unsigned ZF			: 1;		// 计算结果为 0 时，此标志位为 1
	unsigned SF			: 1;		// 符号标志，计算结果为负时该标志为 1
	unsigned TF			: 1;		// 陷阱标志，此标志为 1 时，CPU 每次仅会执行 1 条指令
	unsigned IF			: 1;		// 中断标志，为 0 时禁止响应（屏蔽中断），为 1 时恢复
	unsigned DF			: 1;		// 方向标志
	unsigned OF			: 1;		// 溢出标志，计算结果超出机器表达范围时为1，否则为0
	unsigned IOPL		: 2;		// 用于标明当前任务的 I/O 特权级
	unsigned NT			: 1;		// 任务嵌套标志
	unsigned Reserve4	: 1;
	unsigned RF			: 1;		// 调试异常相应控制标志位，为 1 禁止响应指令断点异常
	unsigned VM			: 1;		// 为 1 时启用虚拟8086模式
	unsigned AC			: 1;		// 内存对齐检查标志
	unsigned VIF		: 1;		// 虚拟中断标志
	unsigned VIP		: 1;		// 虚拟中断标志
	unsigned ID			: 1;		// CPUID检查标志
	unsigned Reserve5	: 10;
}EFLAGS, *PEFLAGS;

class EFlags
{
public:
	static void ShowRegisters(HANDLE hThread);										// 显示寄存器
	static void MemRegisters(HANDLE hThread, char* eFlags, DWORD dwNum);			// 修改通用寄存器
	static void MemEFlagsRegisters(HANDLE hThread, char* eFlags, DWORD dwNum);		// 修改标志寄存器
};