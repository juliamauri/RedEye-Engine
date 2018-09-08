#include "TimeManager.h"

#include "SDL2\include\SDL.h"
#include "ImGui\imgui.h"

TimeManager::TimeManager(float max_fps)
{
	dt = 0.f;
	capped_fps = max_fps;
	capped_ms = 1000.f / capped_fps;
	frames_counter = fps_counter = last_fps_count = last_ms_count = 0u;
	pause_plotting = false;

	ClearArrays();
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

	last_ms_count = ms_timer.Read();

	// cap fps
	if (capped_ms > 0 && last_ms_count < capped_ms)
		SDL_Delay(capped_ms - last_ms_count);
}

void TimeManager::DrawEditor()
{
	if (!pause_plotting)
	{
		for (int i = 0; i < 99; i++)
		{
			ms[i] = ms[i + 1];
			fps[i] = fps[i + 1];
		}

		ms[99] = last_ms_count;
		fps[99] = last_fps_count;
	}

	//ImGui::PushItemWidth(250);
	if (ImGui::SliderFloat("Max FPS", &capped_fps, 20.0f, 144.0f, "%.1f"))
		SetMaxFPS(capped_fps);

	char title[25];
	sprintf_s(title, 25, "Framerate %.1f", fps[99]);
	ImGui::PlotHistogram("##framerate", fps, ((int)(sizeof(fps) / sizeof(*fps))), 0, title, 0.0f, 150.0f, ImVec2(310, 100));
	sprintf_s(title, 25, "Milliseconds %.1f", ms[99]);
	ImGui::PlotHistogram("##milliseconds", ms, ((int)(sizeof(ms) / sizeof(*ms))), 0, title, 0.0f, 40.0f, ImVec2(310, 100));

	if (ImGui::Checkbox(pause_plotting ? "Restart Plotting" : "Pause Plotting", &pause_plotting))
	{
		if (!pause_plotting)
			ClearArrays();
	}
}

void TimeManager::SetMaxFPS(float max_fps)
{
	capped_fps = max_fps;

	if (capped_fps == 0.f)
		capped_ms = 0u;
	else
		capped_ms = 1000.f / capped_fps;
}

float TimeManager::GetMaxFPS() const { return capped_fps; }
float TimeManager::GetDeltaTime() const { return dt; }
unsigned int TimeManager::GetCappedMS() const { return capped_ms; }
unsigned int TimeManager::GetFpsCounter() const { return fps_counter; }
unsigned int TimeManager::GetLastMs() const { return last_ms_count; }
unsigned int TimeManager::GetLastFPS() const { return last_fps_count; }

void TimeManager::ClearArrays()
{
	for (int i = 0; i < 99; i++)
	{
		fps[i] = 0.f;
		ms[i] = 0.f;
	}
}


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
