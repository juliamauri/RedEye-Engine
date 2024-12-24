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

#include "RE_Application.h"

#include "RE_Render.h"

#include <SDL2/SDL.h>
#include <iostream>

bool InitModules();
bool StartModules();
void LoadConfig();
void SaveConfig();

bool Application::Init(int _argc, char* _argv[])
{
    argc = _argc;
    argv = _argv;

    std::cout << "Initializing Application" << std::endl;
    if (SDL_Init(0) != 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "Initializing Modules" << std::endl;
    if (!InitModules())
    {
        std::cerr << "Application Init failed to Initialize Modules" << std::endl;
        SDL_Quit();
        return false;
    }

    std::cout << "Starting Modules" << std::endl;
    if (!StartModules())
    {
        std::cerr << "Application Init failed to Start Modules" << std::endl;
        SDL_Quit();
        return false;
    }

    return true;
}

void Application::MainLoop()
{
    do
    {
        Renderer::Update();

        SDL_Event event;
        while (SDL_PollEvent(&event)) { }

        if (HasFlag(Flag::LOAD_CONFIG))
            LoadConfig();
        if (HasFlag(Flag::SAVE_CONFIG))
            SaveConfig();

    } while (!HasFlag(Flag::WANT_TO_QUIT));
}

void Application::CleanUp()
{
    std::cout << "Quiting Application" << std::endl;
    Renderer::CleanUp();
    SDL_Quit();
}

bool InitModules()
{
    if (!Renderer::Init())
    {
        std::cerr << "Failed to initialize Renderer Module" << std::endl;
        return false;
    }
    return true;
}

bool StartModules()
{
    return true;
}

void LoadConfig()
{
    Application::RemoveFlag(Application::Flag::LOAD_CONFIG);
}

void SaveConfig()
{
    Application::RemoveFlag(Application::Flag::SAVE_CONFIG);
}