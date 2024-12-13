#ifndef __EVENT_LISTENER__
#define __EVENT_LISTENER__

struct Event;

class EventListener
{
public:
	EventListener() = default;
	virtual ~EventListener() = default;
	virtual void RecieveEvent(const Event& e) = 0;
};

#endif // !__EVENT_LISTENER__