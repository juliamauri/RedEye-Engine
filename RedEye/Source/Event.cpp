#include "Event.h"
#include "EventListener.h"
//#include "Application.h"

void RE_Event::CheckTime()
{
	//timestamp = App->GetTicks();
}
Event::Event(EventListener* lis, RE_EventType t) : listener(lis), type(t)
{
	if (lis != nullptr)
		data.re_event.CheckTime();
	else
		SetInvalid();
}

Event::Event(EventListener* lis, SDL_Event* e) : listener(lis), type(SDL_EVENT)
{
	if (lis != nullptr && e != nullptr)
		data.sdl_event = e;
	else
		SetInvalid();
}

Event::~Event()
{
	SetInvalid();
}

void Event::CallListener()
{
	if(listener != nullptr) listener->RecieveEvent(this);
}

bool Event::ValidEvent()
{
	return type != MAX_EVENT_TYPES;
}

void Event::SetInvalid()
{
	data.sdl_event = nullptr;
	type = MAX_EVENT_TYPES;
	listener = nullptr;
}