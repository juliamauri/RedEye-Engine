#include "RE_Time.h"

#include "SDL2\include\SDL_timer.h"
#include "ImGui\imgui.h"
#include <EAStdC/EASprintf.h>

RE_Time::RE_Time()
{
	game_timer.Stop();
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

	if (ImGui::SliderFloat("Max FPS", &capped_fps, 0.0f, 500.0f, "%.1f"))
		SetMaxFPS(capped_fps);

	char title[25];
	EA::StdC::Snprintf(title, 25, "Framerate %.1f", fps[99]);
	ImGui::PlotHistogram("##framerate", fps, ((int)(sizeof(fps) / sizeof(*fps))), 0, title, 0.0f, capped_fps < 1.f ? 500.f : capped_fps, ImVec2(310, 100));
	EA::StdC::Snprintf(title, 25, "Milliseconds %.1f", ms[99]);
	ImGui::PlotHistogram("##milliseconds", ms, ((int)(sizeof(ms) / sizeof(*ms))), 0, title, 0.0f, 40.0f, ImVec2(310, 100));

	if (ImGui::Checkbox(pause_plotting ? "Restart Plotting" : "Pause Plotting", &pause_plotting) && !pause_plotting)
		for (int i = 0; i <= 99; i++)
			fps[i] = ms[i] = 0.f;
}

void RE_Time::Delay(unsigned int ms) const { SDL_Delay(ms); }

unsigned int RE_Time::GetTicks() const { return SDL_GetTicks(); }

float RE_Time::GetEngineTimer() const { return static_cast<float>(SDL_GetTicks()) / 1000.f; }
