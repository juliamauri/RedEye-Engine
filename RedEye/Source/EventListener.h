#ifndef __EVENTLISTENER__
#define __EVENTLISTENER__

#include "Event.h"

class EventListener
{
public:
	virtual void RecieveEvent(const Event& e) {}
};

#endif // !__EVENTLISTENER__
