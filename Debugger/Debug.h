#include <Windows.h>
#include "Memory.h"

class Debug
{
private:
	DEBUG_EVENT					m_debugEvent = { 0 };							// 保存调试信息的结构体
	DWORD						m_continueStatus = DBG_CONTINUE;				// 每一次返回给调试子系统的处理结果
	HANDLE						m_hThread = NULL;								// 产生异常的线程句柄
	HANDLE						m_hProcess = NULL;								// 产生异常的进程句柄
	LPVOID						m_lpOep = nullptr;								// 保存程序的OEP地址
	bool						m_boolIsSystemBreakpoint = true;				// 标识当前是否是系统断点
	bool						m_boolNeedInput = true;
	DWORD						m_dwRemoteImageBase = NULL;
	int							nFlag = 1;
	PCSTR						m_lpFilePath = NULL;
	CREATE_PROCESS_DEBUG_INFO	m_processInfo;
	Memory						m_mMem;

public:
	void	OpenFile(PCSTR lpFilePath);								// 以调试的方式创建进程
	bool	OpenPid(int exepid);									// 通过pid打开exe
	void	Run();													// 接收调试信息
	void	OnExceptHandler();										// 处理接收到的异常调试事件
	void	Input();												// 用户输入
	void	OpenHandles();											// 打开进程和线程句柄
	void	CloseHandles();											// 关闭进程和线程句柄
	void	SetBreakpoint();										// 单步步过
};
