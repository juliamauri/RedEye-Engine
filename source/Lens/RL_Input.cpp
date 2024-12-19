#include "RL_Input.h"

#include <functional>
#include <imgui_impl_sdl2.h>

import EventSystem;

bool JR_Input::Init()
{
    std::function<void(SDL_Event*)> listener = [this](SDL_Event* event) { this->EventListener(event); };
    RE::Event::SetInputListener(listener);
    return true;
}

void JR_Input::EventListener(SDL_Event* event)
{
    ImGui_ImplSDL2_ProcessEvent(event);
}
