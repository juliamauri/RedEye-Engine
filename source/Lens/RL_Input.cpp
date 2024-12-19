#include "RL_Input.h"

#include <functional>
#include <imgui_impl_sdl2.h>
#include <typeinfo>

import EventSystem;

JR_Input* JR_Input::instance = nullptr;

bool JR_Input::Init()
{
    instance = this;
    std::function<void(SDL_Event*)> listener = [](SDL_Event* event) { JR_Input::StaticEventListener(event); };
    RE::Event::SetInputListener(listener);
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
