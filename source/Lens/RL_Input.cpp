/*
 * RedEye Engine - A 3D Game Engine written in C++.
 * Copyright (C) 2018-2024 Julia Mauri and Ruben Sardon
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

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
