#include "RE_TimeManager.h"

#include "OutputLog.h"
#include "SDL2\include\SDL.h"
#include "ImGui\imgui.h"

float RE_TimeManager::dt;
Timer RE_TimeManager::engine_timer(false);
Timer RE_TimeManager::game_timer(false);

RE_TimeManager::RE_TimeManager() : plot_data(new TimePlotting()) {}
RE_TimeManager::~RE_TimeManager() { delete plot_data; }

void RE_TimeManager::Init(float max_fps)
{
	RE_LOG("Initializing Time Manager");
	plot_data->SetMaxFPS(max_fps);
}

float RE_TimeManager::UpdateDeltaTime()
{
	dt = ms_timer.ReadF() / 1000.f;
	ms_timer.Start();
	return dt;
}

unsigned int RE_TimeManager::ManageFrameTimers() { return plot_data->ManageFrameTimers(ms_timer.Read()); }
void RE_TimeManager::DrawEditor() { plot_data->DrawEditor(); }
void RE_TimeManager::SetMaxFPS(float max_fps) { plot_data->SetMaxFPS(max_fps); }

float RE_TimeManager::GetMaxFPS() const { return plot_data->capped_fps; }
float RE_TimeManager::GetDeltaTime() { return dt; }

unsigned int RE_TimeManager::GetCappedMS() const { return plot_data->capped_ms; }
unsigned int RE_TimeManager::GetFpsCounter() const { return plot_data->fps_counter; }
unsigned int RE_TimeManager::GetLastMs() const { return plot_data->last_ms_count; }
unsigned int RE_TimeManager::GetLastFPS() const { return plot_data->last_fps_count; }

void RE_TimeManager::Delay(unsigned int ms) { SDL_Delay(ms); }

float RE_TimeManager::GetEngineTimer() { return engine_timer.ReadF()/1000.f; }
float RE_TimeManager::GetGameTimer() { return game_timer.ReadF()/1000.f; }

void RE_TimeManager::StartGameTimer() { game_timer.Start(); }
void RE_TimeManager::PauseGameTimer() { game_timer.Pause(); }
void RE_TimeManager::StopGameTimer() { game_timer.Stop(); }


// TIMER =======================================================================================
Timer::Timer(const bool start_active) : paused(!start_active) { start_active ? Start() : Stop(); }
Timer::Timer(const Timer & timer) : paused(timer.paused), started_at(timer.started_at), paused_at(timer.paused_at) {}

void Timer::Start()
{
	if (paused)
	{
		started_at = SDL_GetTicks() - (paused_at - started_at);
		paused = false;
	}
	else started_at = SDL_GetTicks();

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
	paused = true;
	started_at = paused_at = 0u;
}

unsigned int Timer::Read() const { return paused ? paused_at - started_at : SDL_GetTicks() - started_at; }
float Timer::ReadF() const { return static_cast<float>(Read()); }


// TIME PLOTTING =======================================================================================
TimePlotting::TimePlotting()
{
	capped_ms = static_cast<unsigned int>(1000.f / capped_fps);
	ClearArrays();
}

void TimePlotting::DrawEditor()
{
	if (!pause_plotting)
	{
		for (int i = 0; i < 99; i++)
		{
			ms[i] = ms[i + 1];
			fps[i] = fps[i + 1];
		}

		ms[99] = static_cast<float>(last_ms_count);
		fps[99] = static_cast<float>(last_fps_count);
	}

	if (ImGui::SliderFloat("Max FPS", &capped_fps, 1.0f, 500.0f, "%.1f"))
		SetMaxFPS(capped_fps);

	char title[25];
	sprintf_s(title, 25, "Framerate %.1f", fps[99]);
	ImGui::PlotHistogram("##framerate", fps, ((int)(sizeof(fps) / sizeof(*fps))), 0, title, 0.0f, capped_fps, ImVec2(310, 100));
	sprintf_s(title, 25, "Milliseconds %.1f", ms[99]);
	ImGui::PlotHistogram("##milliseconds", ms, ((int)(sizeof(ms) / sizeof(*ms))), 0, title, 0.0f, 40.0f, ImVec2(310, 100));

	if (ImGui::Checkbox(pause_plotting ? "Restart Plotting" : "Pause Plotting", &pause_plotting) && !pause_plotting)
			ClearArrays();
}

unsigned int TimePlotting::ManageFrameTimers(unsigned int ms_count)
{
	++frames_counter;
	++fps_counter;

	if (fps_timer.Read() >= 1000)
	{
		last_fps_count = fps_counter;
		fps_counter = 0;
		fps_timer.Start();
	}

	last_ms_count = ms_count;

	unsigned int ret = 1u;
	if (capped_ms > 0 && capped_ms > last_ms_count) ret = capped_ms - last_ms_count;

	return ret;
}

void TimePlotting::SetMaxFPS(float max_fps)
{
	capped_fps = max_fps;
	if (capped_fps == 0.f) capped_ms = 0u;
	else capped_ms = static_cast<unsigned int>(1000.f / capped_fps);
}

void TimePlotting::ClearArrays() { for (int i = 0; i <= 99; i++) fps[i] = ms[i] = 0.f; }