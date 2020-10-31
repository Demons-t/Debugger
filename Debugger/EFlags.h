#pragma once
#include <Windows.h>

// ��־�Ĵ���λ����Ϣ�ṹ��
typedef struct _EFLAGS
{
	unsigned CF			: 1;		// ��λ���λ
	unsigned Reservel	: 1;
	unsigned PF			: 1;		// ��������λ����ż������ 1 ʱ���˱�־Ϊ 1
	unsigned Reserve2	: 2;
	unsigned AF			: 1;		// ������λ��־����λ 3 ���н�λ���λʱ�ñ�־λ1
	unsigned Reserve3	: 3;
	unsigned ZF			: 1;		// ������Ϊ 0 ʱ���˱�־λΪ 1
	unsigned SF			: 1;		// ���ű�־��������Ϊ��ʱ�ñ�־Ϊ 1
	unsigned TF			: 1;		// �����־���˱�־Ϊ 1 ʱ��CPU ÿ�ν���ִ�� 1 ��ָ��
	unsigned IF			: 1;		// �жϱ�־��Ϊ 0 ʱ��ֹ��Ӧ�������жϣ���Ϊ 1 ʱ�ָ�
	unsigned DF			: 1;		// �����־
	unsigned OF			: 1;		// �����־������������������ﷶΧʱΪ1������Ϊ0
	unsigned IOPL		: 2;		// ���ڱ�����ǰ����� I/O ��Ȩ��
	unsigned NT			: 1;		// ����Ƕ�ױ�־
	unsigned Reserve4	: 1;
	unsigned RF			: 1;		// �����쳣��Ӧ���Ʊ�־λ��Ϊ 1 ��ֹ��Ӧָ��ϵ��쳣
	unsigned VM			: 1;		// Ϊ 1 ʱ��������8086ģʽ
	unsigned AC			: 1;		// �ڴ�������־
	unsigned VIF		: 1;		// �����жϱ�־
	unsigned VIP		: 1;		// �����жϱ�־
	unsigned ID			: 1;		// CPUID����־
	unsigned Reserve5	: 10;
}EFLAGS, *PEFLAGS;

class EFlags
{
public:
	static void ShowRegisters(HANDLE hThread);										// ��ʾ�Ĵ���
	static void MemRegisters(HANDLE hThread, char* eFlags, DWORD dwNum);			// �޸�ͨ�üĴ���
	static void MemEFlagsRegisters(HANDLE hThread, char* eFlags, DWORD dwNum);		// �޸ı�־�Ĵ���
};