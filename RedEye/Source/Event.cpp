#include "Event.h"
#include "EventListener.h"

void RE_Event::CheckTime()
{
	//timestamp = GetTicks();
}

Event::Event()
{
	SetInvalid();
}

Event::Event(RE_EventType t, EventListener* lis) : listener(lis), type(t)
{
	IsValid() ? data.CheckTime() : SetInvalid();
}

Event::Event(const Event& e) : listener(e.listener), type(e.type), data(e.data) {}

Event::~Event()
{
	SetInvalid();
}

void Event::CallListener() const
{
	if(listener != nullptr) listener->RecieveEvent(this);
}

bool Event::IsValid() const
{
	return type != MAX_EVENT_TYPES;
}

void Event::SetInvalid()
{
	type = MAX_EVENT_TYPES;
	listener = nullptr;
}