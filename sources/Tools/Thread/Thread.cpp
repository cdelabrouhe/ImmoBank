#include "Thread.h"
#include <windows.h>
#include "..\..\ProdToolTypes.h"

void Thread::create(UserFuncPtr userFunc, void* userParam, const char* _name, ThreadPriority _priority)
{
	m_startParam.m_threadName = _name;
	m_startParam.m_userFunc = userFunc;
	m_startParam.m_userParam = userParam;

	m_handle = CreateThread(NULL, 64 * 1024, (LPTHREAD_START_ROUTINE)startFunc, &m_startParam, CREATE_SUSPENDED, NULL);
	ASSERT(m_handle != 0);

	int winPrio;
	switch (_priority)
	{
	case TP_IDLE:
		winPrio = THREAD_PRIORITY_IDLE;
		break;
	case TP_LOWEST:
		winPrio = THREAD_PRIORITY_LOWEST;
		break;
	case TP_BELOW_NORMAL:
		winPrio = THREAD_PRIORITY_BELOW_NORMAL;
		break;
	case TP_ABOVE_NORMAL:
		winPrio = THREAD_PRIORITY_ABOVE_NORMAL;
		break;
	case TP_HIGHEST:
		winPrio = THREAD_PRIORITY_HIGHEST;
		break;
	case TP_TIME_CRITICAL:
		winPrio = THREAD_PRIORITY_TIME_CRITICAL;
		break;
	case TP_NORMAL:
	default:
		winPrio = THREAD_PRIORITY_NORMAL;
		break;
	}
	SetThreadPriority(m_handle, winPrio);

	int ret = SetThreadAffinityMask(m_handle, 0xFFFFFFFF);

	ASSERT(ret != 0);

	ResumeThread(m_handle);
}

void Thread::sleep(unsigned int milliseconds)
{
	Sleep(milliseconds);
}

unsigned int Thread::startFunc(void* param)
{
	StartFuncParam* startParam = (StartFuncParam*)param;
	unsigned int res = (*startParam->m_userFunc)(startParam->m_userParam);
	return res;
}