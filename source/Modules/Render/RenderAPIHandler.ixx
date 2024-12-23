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

module;

#include <SDL2/SDL.h>

export module Render;

#ifdef ENABLE_OPENGL
import OpenGL;
using namespace RE::OpenGL;
#elif ENABLE_VULKAN
import Vulkan;
using namespace RE::Vulkan;
#endif // DEBUG

export namespace RE
{
    namespace Render
    {
        void* CreateContext(SDL_Window* window)
        {
            return API::CreateContext(window);
        }
        void DeleteContext(void* context)
        {
            API::DeleteContext(context);
        }

        void PrepareRender()
        {
            API::PrepareRender();
        }

        void Render()
        {
            API::Render();
        }

        void SwapWindow(SDL_Window* window)
        {
            API::SwapWindow(window);
        }
    }
} // namespace RE