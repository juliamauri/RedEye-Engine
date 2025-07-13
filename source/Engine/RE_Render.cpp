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

#include "RE_Render.h"

#include <SDL3/SDL.h>
#include <cstdint>

import WindowManager;
import Render;

uint32_t window_sdl = 0;
uint32_t window_gl = 0;
uint32_t window_vk = 0;

bool Renderer::Init()
{
    if (RE::Render::Init() == false)
    {
        return false;
    }

    auto sdl = RE::Window::NewWindow("RedEye Engine SDL", 100, 100, 250, 250, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (window_sdl == -1)
    {
        return false;
    }
    window_sdl = sdl.first;
    if (RE::Render::CreateContext(window_sdl, sdl.second, 250, 250, RE::Render::Flag::DEFAULT) == false)
    {
        return false;
    }

    auto gl =  RE::Window::NewWindow("RedEye Engine OpenGL", 100, 350, 250, 250, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (window_gl == -1)
    {
        return false;
    }
    window_gl = gl.first;
    if (RE::Render::CreateContext(window_gl, gl.second, 250, 250, RE::Render::Flag::OpenGL | RE::Render::Flag::DEFAULT) == false)
    {
        return false;
    }

    auto vulkan  =  RE::Window::NewWindow("RedEye Engine Vulkan", 100, 600, 250, 250, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (window_vk == -1)
    {
        return false;
    }
    window_vk = vulkan.first;
    if (RE::Render::CreateContext(window_vk, vulkan.second, 250, 250, RE::Render::Flag::Vulkan | RE::Render::Flag::DEFAULT) == false)
    {
        return false;
    }

    return  true;
}

bool Renderer::Update()
{
    return
        RE::Render::RenderTriangle(window_sdl) &&
        RE::Render::RenderTriangle(window_gl) &&
        RE::Render::RenderTriangle(window_vk);
}

void Renderer::CleanUp()
{
    RE::Render::CleanUp();
}
