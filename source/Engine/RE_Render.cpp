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

#include <cstdint>

import Render;

uint32_t window_sdl = 0;
uint32_t window_gl = 0;
uint32_t window_vk = 0;

bool Renderer::Init()
{
    return RE::Render::Init()
        && RE::Render::CreateWindow(window_sdl, "RedEye Engine SDL", RE::Render::Flag::DEFAULT) 
        && RE::Render::CreateWindow(window_gl, "RedEye Engine OpenGL", RE::Render::Flag::OpenGL | RE::Render::Flag::DEFAULT) 
        && RE::Render::CreateWindow(window_vk, "RedEye Engine Vulkan", RE::Render::Flag::Vulkan | RE::Render::Flag::DEFAULT)
        ;
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
