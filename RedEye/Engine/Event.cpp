#include "Event.h"

#include "EventListener.h"
#include <SDL2/SDL_timer.h>

Event::Event(RE_EventType t, EventListener * lis, RE_Cvar d1, RE_Cvar d2) :
	type(t), listener(lis), data1(d1), data2(d2), timestamp(SDL_GetTicks()) {}

Event::Event(Event& e) :
	type(e.type), listener(e.listener), data1(e.data1), data2(e.data2), timestamp(SDL_GetTicks()) {}


bool Event::operator==(const Event& other) const
{
	return type == other.type && listener == other.listener && data1 == other.data1 && data2 == other.data2 && timestamp == other.timestamp;
}
