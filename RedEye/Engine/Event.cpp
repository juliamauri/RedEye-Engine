#include "Event.h"

#include "EventListener.h"
#include <SDL2/SDL_timer.h>

Event::Event(RE_EventType t, EventListener * lis, RE_Cvar d1, RE_Cvar d2) :
	type(t), listener(lis), data1(d1), data2(d2), timestamp(SDL_GetTicks()) {}

Event::Event(Event& e) :
	type(e.type), listener(e.listener), data1(e.data1), data2(e.data2), timestamp(SDL_GetTicks()) {}
