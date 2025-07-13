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

#include <SDL3/SDL.h>
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
                renderer = SDL_CreateRenderer(window, nullptr);
                return renderer != nullptr;
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
                SDL_RenderLine(renderer, 200, 100, 100, 300);
                SDL_RenderLine(renderer, 100, 300, 300, 300);
                SDL_RenderLine(renderer, 300, 300, 200, 100);
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
                if (context.gl.Create(ptr) == false)
                {
                    std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
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
                if (context.vk.Create(ptr, w, h)== false)
                {
                    std::cerr << "Failed to create Vulkan context." << std::endl;
                    return false;
                }
#endif
                break;
            default:
                std::cout << "No rendering API specified for window." << std::endl;
                std::cout << "Creating SDL renderer." << std::endl;
                if (context.sdl.Create(ptr) == false)
                {
                    std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
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
                context.gl.RenderTriangle(ptr);
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
            const uint32_t DEFAULT = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        } // namespace Flag

        bool Init()
        {
            std::cout << "Initializing Redeye Render." << std::endl;
            std::cout << "Initializing SDL_VIDEO." << std::endl;
            if (SDL_Init(SDL_INIT_VIDEO)  == false)
            {
                std::cout << "Failed to Initialize SDL_VIDEO: " << SDL_GetError() << std::endl;
                return false;
            }
#ifdef ENABLE_VULKAN
            std::cout << "Initializing Vulkan." << std::endl;
            if (!RE::Vulkan::Init())
            {
                std::cerr << "Failed to initialize Vulkan." << std::endl;
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
        }

        bool CreateContext(uint32_t window_id, SDL_Window* pWindow, int width, int heigh, uint32_t flags = Flag::DEFAULT)
        {
            std::cout << "Creating Context for Window" << std::endl;
            if (!_windows[window_id].CreateContext(pWindow, flags, width, heigh))
            {
                std::cerr << "Failed to create context for window" << std::endl;
                _windows.erase(window_id);
                return false;
            }
            std::cout << "Window: ready to render!" << std::endl;
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