#ifndef __RE_THREADDEFINES_H__
#define __RE_THREADDEFINES_H__

#include <eathread/eathread_mutex.h>

template<class T>
class BeautyMutex : EA::Thread::AutoMutex
{
public:
	BeautyMutex(EA::Thread::Mutex& _m, T& val_input, T& val_output) : EA::Thread::AutoMutex(_m)
	{
		memcpy(&val_output, &val_input, sizeof(T));
	}
	~BeautyMutex() { }
};

#endif // !__RE_THREADDEFINES_H__