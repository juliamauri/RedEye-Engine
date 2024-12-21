#include "RE_Time.h"

#include "RE_Profiler.h"

#include <SDL2/SDL_timer.h>
#include <ImGui/imgui.h>
#include <EASTL/string.h>

RE_Time::GameState state = RE_Time::GameState::STOP;

// Frame time data
float dt = 0;
float capped_fps = 60.f;
uint capped_ms = 16;
uint last_fps_count = 0;
uint last_ms_count = 0;
uint fps_counter = 0;
ulong global_frame_counter = 0;

// Timers
RE_Timer ms_timer; // read every frame
RE_Timer fps_timer; // read every second
RE_Timer game_timer(false); // read when playing scene

// Plotting
constexpr size_t time_plotting_range = 100;
eastl::array<float, time_plotting_range> fps = {};
eastl::array<float, time_plotting_range> ms = {};
bool pause_time_plotting = false;
float max_fps = 0.f;

// Time Controls
float RE_Time::FrameDeltaTime()
{
	dt = ms_timer.ReadF() / 1000.f;
	ms_timer.Start();
	return dt;
}

uint RE_Time::FrameExtraMS()
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

void RE_Time::Delay(uint ms)
{
	SDL_Delay(ms);
}

void RE_Time::SetMaxFPS(float max_fps)
{
	capped_fps = max_fps;
	if (capped_fps < 1.f) capped_ms = 0;
	else capped_ms = static_cast<uint>(1000.f / capped_fps);
}

// Getters
float RE_Time::MaxFPS() { return capped_fps; }
float RE_Time::DeltaTime() { return dt; }
uint RE_Time::CappedMS() { return capped_ms; }
uint RE_Time::FpsCounter() { return fps_counter; }
uint RE_Time::LastMs() { return last_ms_count; }
uint RE_Time::LastFPS() { return last_fps_count; }
uint RE_Time::Ticks() { return SDL_GetTicks(); }
float RE_Time::EngineTimer() { return static_cast<float>(SDL_GetTicks()) / 1000.f; }
float RE_Time::GameTimer() { return game_timer.ReadF() / 1000.f; }
float RE_Time::CurrentTimer() { return (state == GameState::STOP) ? EngineTimer() : game_timer.ReadF() / 1000.f; }

// Game Timer Controls
void RE_Time::TickGameTimer() { game_timer.Start(); state = GameState::TICK; }
void RE_Time::StartGameTimer() { game_timer.Start(); state = GameState::PLAY; }
void RE_Time::PauseGameTimer() { game_timer.Pause(); state = GameState::PAUSE; }
void RE_Time::StopGameTimer() { game_timer.Stop(); state = GameState::STOP; }

// Game State
RE_Time::GameState RE_Time::State() { return state; }
bool RE_Time::StateIs(RE_Time::GameState s) { return state == s; }

// Editor Draws

RE_Time::GameState RE_Time::DrawEditorControls()
{
	GameState requested_next_state = GameState::NONE;
	if (state <= GameState::PLAY)
	{
		if (ImGui::Button("Restart")) requested_next_state = GameState::PLAY;
		ImGui::SameLine();
		if (ImGui::Button("Pause")) requested_next_state = GameState::PAUSE;
	}
	else
	{
		if (ImGui::Button(" Play  ")) requested_next_state = GameState::PLAY;
		ImGui::SameLine();
		if (ImGui::Button("Tick ")) requested_next_state = GameState::TICK;
	}

	ImGui::SameLine();
	if (ImGui::Button("Stop") && state != GameState::STOP) requested_next_state = GameState::STOP;

	ImGui::SameLine();
	ImGui::Text("%.2f", GameTimer());

	return requested_next_state;
}

void RE_Time::DrawEditorGraphs()
{
	if (!pause_time_plotting)
	{
		max_fps = fps[0];
		for (size_t i = 0; i < time_plotting_range - 1; i++)
		{
			ms[i] = ms[i + 1];
			fps[i] = fps[i + 1];
			if (max_fps < fps[i]) max_fps = fps[i];
		}

		ms[time_plotting_range - 1] = static_cast<float>(last_ms_count);
		fps[time_plotting_range - 1] = static_cast<float>(last_fps_count);
	}

	if (ImGui::SliderFloat("Max FPS", &capped_fps, 0.0f, 500.0f, "%.0f"))
		SetMaxFPS(capped_fps);

	ImGui::PlotHistogram(
		"##framerate",
		fps.begin(),
		time_plotting_range,
		0,
		(eastl::string("Framerate ") + eastl::to_string(static_cast<uint>(fps[time_plotting_range - 1]))).c_str(),
		0.0f,
		(capped_fps < 1.f ? max_fps : capped_fps) * 1.2f,
		ImVec2(310, 100));

	ImGui::PlotHistogram(
		"##milliseconds",
		ms.begin(),
		time_plotting_range,
		0,
		(eastl::string("Milliseconds ") + eastl::to_string(static_cast<uint>(ms[time_plotting_range - 1]))).c_str(),
		0.0f,
		40.0f, 
		ImVec2(310, 100));

#ifdef INTERNAL_PROFILING

	if (ImGui::Checkbox(pause_time_plotting ? "Restart Plotting" : "Pause Plotting", &pause_time_plotting) && !pause_time_plotting)
		for (int i = 0; i < time_plotting_range; i++)
			fps[i] = ms[i] = 0.f;

	if (ProfilingTimer::recording)
	{
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.75, 0.0,0.0,1.0 });
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, { 1.0, 0.0,0.0,1.0 });
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, { 0.5, 0.0,0.0,1.0 });
		if (ImGui::Button("Pause profiling")) RE_Profiler::Pause();
		ImGui::PopStyleColor(3);
	}
	else if (ImGui::Button("Start profiling")) RE_Profiler::Start();

	auto count = ProfilingTimer::operations.size();
	if (count > 0)
	{
		if (ImGui::Button((eastl::string("Deploy ") + eastl::to_string(count)).c_str())) RE_Profiler::Deploy();
		if (ImGui::Button("Clear Logs")) RE_Profiler::Clear();
	}

#endif // INTERNAL_PROFILING
}