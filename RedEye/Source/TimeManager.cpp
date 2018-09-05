#include "TimeManager.h"

#include "SDL2\include\SDL.h"

TimeManager::TimeManager(unsigned int max_fps)
{
	dt = 0.f;
	capped_fps = max_fps;
	capped_ms = 1000 / capped_fps;
	frames_counter = fps_counter = last_fps_count = last_frame_ms = 0u;
}

TimeManager::~TimeManager()
{}

float TimeManager::UpdateDeltaTime()
{
	dt = ms_timer.ReadF() / 1000.f;
	ms_timer.Start();

	return dt;
}

void TimeManager::ManageFrameTimers()
{
	// Recap on framecount and fps
	++frames_counter;
	++fps_counter;

	if (fps_timer.Read() >= 1000)
	{
		last_fps_count = fps_counter;
		fps_counter = 0;
		fps_timer.Start();
	}

	last_frame_ms = ms_timer.Read();

	// cap fps
	if (capped_ms > 0 && last_frame_ms < capped_ms)
		SDL_Delay(capped_ms - last_frame_ms);
}

void TimeManager::SetMaxFPS(unsigned int max_fps)
{
	capped_fps = max_fps;

	if (capped_fps == 0)
		capped_ms = 0;
	else
		capped_ms = 1000 / capped_fps;
}

float TimeManager::GetMaxFPS() const { return capped_fps; }
float TimeManager::GetDeltaTime() const { return dt; }
unsigned int TimeManager::GetCappedMS() const { return capped_ms; }
unsigned int TimeManager::GetFpsCounter() const { return fps_counter; }
unsigned int TimeManager::GetLastFrameMs() const { return last_frame_ms; }
unsigned int TimeManager::GetLastFPS() const { return last_fps_count; }


// TIME =======================================================================================
Timer::Timer(const bool start_active) : paused(!start_active)
{
	start_active ? Start() : Stop();
}

Timer::Timer(const Timer & timer) :
	paused(timer.paused),
started_at(timer.started_at),
paused_at(timer.paused_at)
{}

void Timer::Start()
{
	if (paused)
	{
		started_at = SDL_GetTicks() - (paused_at - started_at);
		paused = false;
	}
	else
	{
		started_at = SDL_GetTicks();
	}

	paused_at = 0u;
}

void Timer::Pause()
{
	if (!paused)
	{
		paused = true;
		paused_at = SDL_GetTicks();
	}
}

void Timer::Stop()
{
	paused = false;
	started_at = paused_at = 0u;
}

unsigned int Timer::Read() const
{
	return paused ? paused_at - started_at : SDL_GetTicks() - started_at;
}

float Timer::ReadF() const
{
	return (float)Read();
}
