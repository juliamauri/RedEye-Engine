#ifndef __RE_THREAD_H__
#define __RE_THREAD_H__

#include "RE_ThreadDefines.h"
#include <eathread/eathread_thread.h>


class RE_Thread : public EA::Thread::IRunnable
{
public:
	virtual ~RE_Thread() { }

	intptr_t Run(void* pContext = NULL)
	{
		bool _stop = false;

		while (!_stop) {
			RunJob(pContext);
			BeautyMutex<bool> bm(_finish_mutex, _finish, _stop);
		}
		return 0;
	}

	bool neededEnd()
	{
		EA::Thread::AutoMutex a(_finish_mutex);
		return _finish;
	}

	void Finish() {
		EA::Thread::AutoMutex a(_finish_mutex);
		_finish = true;
	}
private:
	virtual void RunJob(void* pContext = NULL) = 0;

private:
	EA::Thread::Mutex _finish_mutex;
	bool _finish = false;
};

#endif // !__RE_THREAD_H__