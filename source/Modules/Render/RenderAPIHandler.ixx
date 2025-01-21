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

export module Render;

#ifdef ENABLE_OPENGL
import OpenGL;
#endif
#ifdef ENABLE_VULKAN
import Vulkan;
#endif

struct Window
{
    SDL_Window* ptr;
    
    enum API
    {
        SDL,
        OpenGL,
        Vulkan
    } type;

    union Context
    {
        struct SDLContext
        {
            SDL_Renderer* renderer = nullptr;

            bool Create(SDL_Window* window)
            {
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
                return renderer != NULL;
            }

            void Delete()
            {
                SDL_DestroyRenderer(renderer);
            }

            void RenderTriangle()
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawLine(renderer, 200, 100, 100, 300);
                SDL_RenderDrawLine(renderer, 100, 300, 300, 300);
                SDL_RenderDrawLine(renderer, 300, 300, 200, 100);
                SDL_RenderPresent(renderer);
            }
        } sdl;

#ifdef ENABLE_OPENGL
        RE::OpenGL::Context gl;
#endif
#ifdef ENABLE_VULKAN
        RE::Vulkan::Context vk;
#endif
        int i = 0;
        ~Context() {}
    } context = {0};

    bool CreateContext(SDL_Window* sdl_window, uint32_t flags, int w, int h)
    {
        ptr = sdl_window;

        if (flags & SDL_WINDOW_OPENGL)
            type = OpenGL;
        else if (flags & SDL_WINDOW_VULKAN)
            type = Vulkan;
        else
            type = SDL;

        switch (type)
        {
            case Window::OpenGL:
#ifndef ENABLE_OPENGL
                std::cerr << "OpenGL is not enabled." << std::endl;
                SDL_DestroyWindow(ptr);
                return false;
#else
                std::cout << "Creating OpenGL context." << std::endl;
                if (!context.gl.Create(ptr))
                {
                    std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
                    SDL_DestroyWindow(ptr);
                    return false;
                }
#endif
                break;
            case Window::Vulkan:
#ifndef ENABLE_VULKAN
                std::cerr << "Vulkan is not enabled." << std::endl;
                SDL_DestroyWindow(ptr);
                return false;
#else
                std::cout << "Creating Vulkan context." << std::endl;
                if (!context.vk.Create(ptr, w, h))
                {
                    std::cerr << "Failed to create Vulkan context." << std::endl;
                    SDL_DestroyWindow(ptr);
                    return false;
                }
#endif
                break;
            default:
                std::cout << "No rendering API specified for window." << std::endl;
                std::cout << "Creating SDL renderer." << std::endl;
                if (!context.sdl.Create(ptr))
                {
                    std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
                    SDL_DestroyWindow(ptr);
                    return false;
                }
                break;
        }

        std::cout << "Successfull context creation for window" << std::endl;
        return true;
    }

    void Delete()
    {
        switch (type)
        {
#ifdef ENABLE_OPENGL
            case Window::OpenGL:
                context.gl.Delete();
                break;
#endif
#ifdef ENABLE_VULKAN
            case Window::Vulkan:
                context.vk.Delete();
                break;
#endif
            default:
                context.sdl.Delete();
                break;
        }
        SDL_DestroyWindow(ptr);
    }

    bool RenderTriangle()
    {
        switch (type)
        {
            case SDL:
                context.sdl.RenderTriangle();
                return true;
#ifdef ENABLE_OPENGL
            case Window::OpenGL:
                context.gl.RenderTriangle();
                SDL_GL_SwapWindow(ptr);
                return true;
#endif
#ifdef ENABLE_VULKAN
            case Window::Vulkan:
                return context.vk.RenderTriangle();
#endif
            default:
                return false;
        }
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
            std::cout << "Initializing Vulkan." << std::endl;
            if (!RE::Vulkan::Init())
            {
                std::cerr << "Failed to initialize Vulkan." << std::endl;
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
            std::cout << "Creating SDL Window: " << title << std::endl;
            SDL_Window* window_ptr = SDL_CreateWindow(title, x, y, w, h, flags);
            if (window_ptr == nullptr)
            {
                std::cerr << "Failed to create SDL Window: " << SDL_GetError() << std::endl;
                return false;
            }

            std::cout << "Creating Context for Window: " << title << " " << std::endl;
            out_window_id = SDL_GetWindowID(window_ptr);
            if (!_windows[out_window_id].CreateContext(window_ptr, flags, w, h))
            {
                std::cerr << "Failed to create context for window: " << title << std::endl;
                _windows.erase(out_window_id);
                return false;
            }

            std::cout << "Window: " << title << " ready to render!" << std::endl;
            return true;
        }

        void DeleteWindow(uint32_t window_id)
        {
            _windows[window_id].Delete();
            _windows.erase(window_id);
        }

        bool RenderTriangle(uint32_t window_id)
        {
            return _windows[window_id].RenderTriangle();
        }
    } // namespace Render
} // namespace RE