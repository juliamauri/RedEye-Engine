#include "Event.h"
#include "EventListener.h"

Event::Event()
{
	Clear();
}

Event::Event(RE_EventType t, unsigned int ts, EventListener* lis) : listener(lis), type(t), timestamp(ts)
{
	if (!IsValid())
	{
		Clear();
	}
}

Event::Event(const Event& e) : listener(e.listener), type(e.type), timestamp(e.timestamp) {}

Event::~Event()
{
	Clear();
}

void Event::CallListener() const
{
	if(listener != nullptr) listener->RecieveEvent(this);
}

bool Event::IsValid() const
{
	return type != MAX_EVENT_TYPES;
}

unsigned int Event::GetTimeStamp() const
{
	return timestamp;
}

RE_EventType Event::GetType() const
{
	return type;
}

void Event::Clear()
{
	type = MAX_EVENT_TYPES;
	listener = nullptr;
	timestamp = 0;
}