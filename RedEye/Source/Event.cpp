#include "Event.h"
#include "EventListener.h"
#include "SDL2\include\SDL_timer.h"

Event::Event(RE_EventType t, EventListener * lis, Cvar d1, Cvar d2)
	: type(t), listener(lis), data1(d1), data2(d2), timestamp(SDL_GetTicks())
{}

Event::Event(const Event& e)
	: type(e.type), listener(e.listener), data1(e.data1), data2(e.data2), timestamp(SDL_GetTicks())
{}

Event::~Event()
{
	Clear();
}

void Event::CallListener() const
{
	if(listener != nullptr) listener->RecieveEvent(*this);
}

bool Event::IsValid() const
{
	return type < MAX_EVENT_TYPES && listener != nullptr;
}

unsigned int Event::GetTimeStamp() const
{
	return timestamp;
}

RE_EventType Event::GetType() const
{
	return type;
}

const Cvar& Event::GetData() const
{
	return data1;
}

const Cvar & Event::GetDataNext() const
{
	return data2;
}

void Event::Push(RE_EventType t, EventListener * lis, Cvar d1, Cvar d2)
{
	events_queue.push(Event(t, lis, d1, d2));
}

void Event::PumpAll()
{
	while (!events_queue.empty())
	{
		const Event e = events_queue.front();

		if (e.IsValid())
			e.CallListener();

		Event::events_queue.pop();
	}
}

void Event::Clear()
{
	type = MAX_EVENT_TYPES;
	listener = nullptr;
}