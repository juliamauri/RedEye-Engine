#ifndef __RE_THREADPOOL_HPP__
#define __RE_THREADPOOL_HPP__

class RE_Thread;

namespace RE_ThreadPool 
{
	void Init();
	void CleanUp();
	intptr_t AddThread(RE_Thread* thread);
};

#endif // !__RE_THREADPOOL_HPP__