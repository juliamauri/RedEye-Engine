#include "RE_Thread.h"
#include <eathread/eathread_pool.h>
#include "RE_ThreadPool.hpp"

#include "EASTL/stack.h"

namespace RE_ThreadPool
{
	namespace {
		static EA::Thread::ThreadPool* _pool = nullptr;
		static eastl::stack<RE_Thread*> _threads;
	}

	void Init() { _pool = EA::Thread::ThreadPoolFactory::CreateThreadPool(); }
	void CleanUp()
	{
		while (!_threads.empty())
		{
			_threads.top()->Finish();
			_threads.pop();
		}
		_pool->Shutdown();
		EA::Thread::ThreadPoolFactory::DestroyThreadPool(_pool); 
	}

	intptr_t AddThread(RE_Thread* thread)
	{
		_threads.push(thread);
		return _pool->Begin(dynamic_cast<EA::Thread::IRunnable*>(thread));
	}
};