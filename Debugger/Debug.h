#include <Windows.h>
#include "Memory.h"

class Debug
{
private:
	DEBUG_EVENT					m_debugEvent = { 0 };							// ���������Ϣ�Ľṹ��
	DWORD						m_continueStatus = DBG_CONTINUE;				// ÿһ�η��ظ�������ϵͳ�Ĵ�����
	HANDLE						m_hThread = NULL;								// �����쳣���߳̾��
	HANDLE						m_hProcess = NULL;								// �����쳣�Ľ��̾��
	LPVOID						m_lpOep = nullptr;								// ��������OEP��ַ
	bool						m_boolIsSystemBreakpoint = true;				// ��ʶ��ǰ�Ƿ���ϵͳ�ϵ�
	bool						m_boolNeedInput = true;
	DWORD						m_dwRemoteImageBase = NULL;
	int							nFlag = 1;
	PCSTR						m_lpFilePath = NULL;
	CREATE_PROCESS_DEBUG_INFO	m_processInfo;
	Memory						m_mMem;

public:
	void	OpenFile(PCSTR lpFilePath);								// �Ե��Եķ�ʽ��������
	bool	OpenPid(int exepid);									// ͨ��pid��exe
	void	Run();													// ���յ�����Ϣ
	void	OnExceptHandler();										// ������յ����쳣�����¼�
	void	Input();												// �û�����
	void	OpenHandles();											// �򿪽��̺��߳̾��
	void	CloseHandles();											// �رս��̺��߳̾��
	void	SetBreakpoint();										// ��������
};
