#include "RE_Timer.h"

#include "SDL2/include/SDL_timer.h"

void RE_Timer::Start()
{
	started_at = SDL_GetTicks() - (paused * (paused_at - started_at));
	paused = false;
	paused_at = 0u;
}

void RE_Timer::Pause()
{
	if (!paused) paused_at = SDL_GetTicks();
	paused = true;
}

unsigned int RE_Timer::Read() const { return paused ? paused_at - started_at : SDL_GetTicks() - started_at; }
