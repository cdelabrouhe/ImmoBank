#pragma once

// To avoid including <windows.h> here
#ifndef WINAPI
#define WINAPI __stdcall
#endif
typedef void* HANDLE;

enum ThreadPriority
{
	TP_IDLE,
	TP_LOWEST,
	TP_BELOW_NORMAL,
	TP_NORMAL,
	TP_ABOVE_NORMAL,
	TP_HIGHEST,
	TP_TIME_CRITICAL
};

typedef unsigned int(*UserFuncPtr)(void*);
class Thread
{
private:
	struct StartFuncParam
	{
		UserFuncPtr  m_userFunc;
		void*        m_userParam;
		bool		specificGraphicContext;
		void *hDC, *hRC;
		char const *		m_threadName;
	};

	static unsigned int WINAPI startFunc(void* startParam);

public:
	void create(UserFuncPtr _userFunc, void* _userParam, const char* _name, ThreadPriority _priority = TP_NORMAL);
	static void sleep(unsigned int milliseconds);

private:
	StartFuncParam m_startParam;
	HANDLE m_handle;
};