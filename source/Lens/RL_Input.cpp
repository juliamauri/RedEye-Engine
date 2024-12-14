#include "RL_Input.h"

#include <imgui_impl_sdl2.h>

import EventSystem;

bool JR_Input::Init()
{
	RE::Event::SetInputListener([this](SDL_Event* event) { this->EventListener(event); });
	return true;
}

void JR_Input::EventListener(SDL_Event* event)
{
	ImGui_ImplSDL2_ProcessEvent(event);
}
