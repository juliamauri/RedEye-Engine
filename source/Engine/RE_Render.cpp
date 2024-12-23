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

import WindowManager;
import Render;

uint32_t window = 0;
void* context = nullptr;

bool Renderer::Init()
{
    if (!RE::Window::Init())
        return false;

    window = RE::Window::NewWindow("RedEye Engine");
    context = RE::Render::CreateContext(RE::Window::GetWindow(window));

    return context != nullptr;
}

void Renderer::Update()
{
    RE::Render::PrepareRender();
    RE::Render::Render();
    RE::Render::SwapWindow(RE::Window::GetWindow(window));
}

void Renderer::CleanUp()
{
    RE::Render::DeleteContext(context);
    RE::Window::CleanUp();
}
