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
#include <iostream>
#include <unordered_map>

#ifdef ENABLE_OPENGL
#include <GL/glew.h>
#endif
#ifdef ENABLE_VULKAN
#include <SDL_vulkan.h>
#endif

export module Render;

#ifdef ENABLE_OPENGL
import OpenGL;
#endif
#ifdef ENABLE_VULKAN
import Vulkan;
#endif

struct Window
{
    uint32_t id;

    union Data
    {
        bool is_cpu_rendering = false;
#ifdef ENABLE_OPENGL
        SDL_GLContext gl_context;
#endif
#ifdef ENABLE_VULKAN
        RE::VulkanContext vk_context;
#endif
    } data;

    bool Create(const char* title, uint32_t flags, int x, int y, int w, int h)
    {
        SDL_Window* window = SDL_CreateWindow(title, x, y, w, h, flags);
        if (window == nullptr)
        {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            return false;
        }

        if (flags & SDL_WINDOW_OPENGL)
        {
#ifndef ENABLE_OPENGL
            std::cerr << "OpenGL is not enabled." << std::endl;
            SDL_DestroyWindow(window);
            return false;
#else
            std::cout << "Creating OpenGL context." << std::endl;
            if (!RE::OpenGL::CreateContext(data.gl_context, window))
            {
                std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
                SDL_DestroyWindow(window);
                return false;
            }
#endif
        }
        else if (flags & SDL_WINDOW_VULKAN)
        {
#ifndef ENABLE_VULKAN
            std::cerr << "Vulkan is not enabled." << std::endl;
            SDL_DestroyWindow(window);
            return false;
#else
            std::cout << "Creating Vulkan context." << std::endl;
            if (!data.vk_context.Create(window))
            {
                std::cerr << "Failed to create Vulkan context: " << std::endl;
                SDL_DestroyWindow(window);
                return false;
            }
#endif
        }
        else
        {
            std::cout << "No rendering API specified for window: " << title << std::endl;
            data.is_cpu_rendering = true;
        }

        std::cout << "Successfull context creation for window: " << title << std::endl;
        id = SDL_GetWindowID(window);
        return true;
    };

    void Delete()
    {
        SDL_Window* window = SDL_GetWindowFromID(id);
        uint32_t flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_OPENGL)
            RE::OpenGL::DeleteContext(data.gl_context);
        else if (flags & SDL_WINDOW_VULKAN)
            data.vk_context.Delete();

        SDL_DestroyWindow(window);
    }

    void RenderTriangle()
    {
        SDL_Window* window = SDL_GetWindowFromID(id);
        uint32_t flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_OPENGL)
            RE::OpenGL::RenderTriangle(window);
        else if (flags & SDL_WINDOW_VULKAN)
            data.vk_context.RenderTriangle(window);
    }
};

std::unordered_map<uint32_t, Window> _windows;

export namespace RE
{
    namespace Render
    {
        namespace Flag
        {
            const uint32_t OpenGL = SDL_WINDOW_OPENGL;
            const uint32_t Vulkan = SDL_WINDOW_VULKAN;
            const uint32_t DEFAULT = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN;
        } // namespace Flag

        bool Init()
        {
            std::cout << "Initializing Redeye Render." << std::endl;
            std::cout << "Initializing SDL_VIDEO." << std::endl;
            if (SDL_Init(SDL_INIT_VIDEO) != 0)
            {
                std::cout << "Failed to Initialize SDL_VIDEO: " << SDL_GetError() << std::endl;
                return false;
            }
#ifdef ENABLE_VULKAN
            std::cout << "Loading default Vulkan library." << std::endl;
            if (SDL_Vulkan_LoadLibrary(nullptr) != 0)
            {
                std::cerr << "Failed to load default Vulkan library: " << SDL_GetError() << std::endl;
                SDL_QuitSubSystem(SDL_INIT_VIDEO);
                return false;
            }
#endif
            return true;
        }

        void CleanUp()
        {
            for (auto& window : _windows)
                window.second.Delete();

            _windows.clear();
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }

        bool CreateWindow(uint32_t& out_window_id, const char* title, uint32_t flags = Flag::DEFAULT,
                          int x = SDL_WINDOWPOS_CENTERED, int y = SDL_WINDOWPOS_CENTERED, int w = 500, int h = 500)
        {
            std::cout << "Creating Window: " << title << std::endl;
            Window window;
            if (!window.Create(title, flags, x, y, w, h))
            {
                std::cerr << "Failed to create window: " << title << std::endl;
                return false;
            }

            _windows[out_window_id = window.id] = window;
            return true;
        }

        void DeleteWindow(uint32_t window_id)
        {
            _windows[window_id].Delete();
            _windows.erase(window_id);
        }

        void RenderTriangle(uint32_t window_id)
        {
            _windows[window_id].RenderTriangle();
        }
    } // namespace Render
} // namespace RE