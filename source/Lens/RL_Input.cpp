#include "RL_Input.h"

#include <imgui_impl_sdl2.h>

import EventSystem;

JR_Input* JR_Input::instance = nullptr;

bool JR_Input::Init()
{
    instance = this;
    RE::Event::SetInputListener(StaticEventListener);
    return true;
}

void JR_Input::StaticEventListener(SDL_Event* event)
{
    if (instance)
    {
        instance->EventListener(event);
    }
}

void JR_Input::EventListener(SDL_Event* event)
{
    ImGui_ImplSDL2_ProcessEvent(event);
}
