#include "RE_Time.h"

#include "SDL2\include\SDL_timer.h"
#include "ImGui\imgui.h"
#include <EAStdC/EASprintf.h>

using namespace RE_Time::Internal;

float RE_Time::FrameDeltaTime()
{
	dt = ms_timer.ReadF() / 1000.f;
	ms_timer.Start();
	return dt;
}

unsigned int RE_Time::FrameExtraMS()
{
	++global_frame_counter;
	++fps_counter;

	if (fps_timer.Read() >= 1000)
	{
		last_fps_count = fps_counter;
		fps_counter = 0u;
		fps_timer.Start();
	}

	last_ms_count = ms_timer.Read();

	unsigned int ret = 1u;
	if (capped_ms > 0 && capped_ms > last_ms_count) ret = capped_ms - last_ms_count;

	return ret;
}

GameState RE_Time::DrawEditorControls()
{
	GameState requested_next_state = GS_EMPTY;
	if (state <= GS_PLAY)
	{
		if (ImGui::Button("Restart")) requested_next_state = GS_PLAY;
		ImGui::SameLine();
		if (ImGui::Button("Pause")) requested_next_state = GS_PAUSE;
	}
	else
	{
		if (ImGui::Button(" Play  ")) requested_next_state = GS_PLAY;
		ImGui::SameLine();
		if (ImGui::Button("Tick ")) requested_next_state = GS_TICK;
	}

	ImGui::SameLine();
	if (ImGui::Button("Stop") && state != GS_STOP) requested_next_state = GS_STOP;

	ImGui::SameLine();
	ImGui::Text("%.2f", GetGameTimer());

	return requested_next_state;
}

void RE_Time::DrawEditorGraphs()
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
	EA::StdC::Snprintf(title, 25, "Framerate %.1f", fps[99]);
	ImGui::PlotHistogram("##framerate", fps, ((int)(sizeof(fps) / sizeof(*fps))), 0, title, 0.0f, capped_fps, ImVec2(310, 100));
	EA::StdC::Snprintf(title, 25, "Milliseconds %.1f", ms[99]);
	ImGui::PlotHistogram("##milliseconds", ms, ((int)(sizeof(ms) / sizeof(*ms))), 0, title, 0.0f, 40.0f, ImVec2(310, 100));

	if (ImGui::Checkbox(pause_plotting ? "Restart Plotting" : "Pause Plotting", &pause_plotting) && !pause_plotting)
		for (int i = 0; i <= 99; i++)
			fps[i] = ms[i] = 0.f;
}

void RE_Time::SetMaxFPS(float max_fps)
{
	capped_fps = max_fps;
	if (capped_fps == 0.f) capped_ms = 0u;
	else capped_ms = static_cast<unsigned int>(1000.f / capped_fps);
}

float RE_Time::GetMaxFPS() { return capped_fps; }
float RE_Time::GetDeltaTime() { return dt; }

unsigned int RE_Time::GetCappedMS() { return capped_ms; }
unsigned int RE_Time::GetFpsCounter() { return fps_counter; }
unsigned int RE_Time::GetLastMs() { return last_ms_count; }
unsigned int RE_Time::GetLastFPS() { return last_fps_count; }

void RE_Time::Delay(unsigned int ms) { SDL_Delay(ms); }

float RE_Time::GetEngineTimer() { return static_cast<float>(SDL_GetTicks()) / 1000.f; }
float RE_Time::GetGameTimer() { return game_timer.ReadF()/1000.f; }
float RE_Time::GetCurrentTimer() { return (state == GS_STOP) ? GetEngineTimer() : GetGameTimer(); }

GameState RE_Time::GetState() { return state; }

void RE_Time::TickGameTimer() { game_timer.Start(); state = GS_TICK; }
void RE_Time::StartGameTimer() { game_timer.Start(); state = GS_PLAY; }
void RE_Time::PauseGameTimer() { game_timer.Pause(); state = GS_PAUSE; }
void RE_Time::StopGameTimer() { game_timer.Stop(); state = GS_STOP; }

// TIMER =======================================================================================

Timer::Timer(const bool start_active) { start_active ? Start() : Stop(); }
Timer::Timer(const Timer & timer) :
	paused(timer.paused),
	started_at(timer.started_at),
	paused_at(timer.paused_at) {}

void Timer::Start()
{
	started_at = SDL_GetTicks() - (paused * (paused_at - started_at));
	paused = false;
	paused_at = 0u;
}

void Timer::Pause()
{
	if (!paused) paused_at = SDL_GetTicks();
	paused = true;
}

void Timer::Stop()
{
	paused = true;
	started_at = paused_at = 0u;
}

unsigned int Timer::Read() const { return paused ? paused_at - started_at : SDL_GetTicks() - started_at; }
float Timer::ReadF() const { return static_cast<float>(Read()); }
