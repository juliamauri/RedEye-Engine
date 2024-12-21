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

#include "RL_Application.h"

#include "RL_Input.h"
#include "RL_WindowAndRenderer.h"

#include "RL_Projects.h"

#include <SDL2/SDL.h>
#include <functional>

import FileSystem;
import EventSystem;
import WindowManager;
import Dialogs;
import GUIManager;

JR_Application* JR_Application::App = nullptr;

bool JR_Application::Init(char* argv[])
{
    std::function<void(SDL_Event*)> listener = [this](SDL_Event* event) { this->EventListener(event); };
    if (SDL_Init(0) == 0 && RE::FileSystem::Init(argv, "RedEye", "Lens") && RE::Event::Init(listener) &&
        (input = new JR_Input())->Init() && (visual_magnament = new JR_WindowAndRenderer())->Init() &&
        RE::Dialogs::Init() && RE::GUI::Init(visual_magnament->GetMainWindow(), visual_magnament->GetContext()) &&
        (projects_manager = new RL_Projects())->Init())
        return true;

    CleanUp();
    return false;
}

void JR_Application::Update()
{
    while (true)
    {
        switch (status)
        {
            case JR_Application::Status::RUNNING:
                RE::Event::PumpEvents();
                visual_magnament->PostUpdate();
                break;
            case JR_Application::Status::WANT_EXIT:
            default:
                return;
                break;
        }
    }
}

void JR_Application::CleanUp()
{
    RE::GUI::CleanUp();
    RE::Dialogs::Quit();
    if (visual_magnament)
    {
        visual_magnament->CleanUp();
        delete visual_magnament;
    }
    RE::Window::CleanUp();

    if (input)
        delete input;

    if (projects_manager)
    {
        projects_manager->CleanUp();
        delete projects_manager;
    }

    RE::FileSystem::CleanUp();

    SDL_Quit();
}

void JR_Application::EventListener(SDL_Event* event)
{
    if (event->type == SDL_QUIT)
        RequestExit();
}
