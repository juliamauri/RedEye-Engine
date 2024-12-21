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

#include <unordered_map>

export module WindowManager;

std::unordered_map<uint32_t, SDL_Window*> _windows;

export namespace RE
{
    namespace Window
    {
        /**
         * @brief Creates a new SDL window and adds it to the window manager.
         * @param title The title of the window.
         * @param x The x position of the window (default is centered).
         * @param y The y position of the window (default is centered).
         * @param w The width of the window (default is 500).
         * @param h The height of the window (default is 500).
         * @param flags The window flags (default is SDL_WINDOW_OPENGL |
         * SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN).
         * @return The ID of the newly created window.
         */
        uint32_t NewWindow(const char* title, int x = SDL_WINDOWPOS_CENTERED, int y = SDL_WINDOWPOS_CENTERED,
                           int w = 500, int h = 500,
                           uint32_t flags = (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI |
                                             SDL_WINDOW_SHOWN))
        {
            SDL_Window* window = SDL_CreateWindow(title, x, y, w, h, flags);
            uint32_t id = SDL_GetWindowID(window);
            _windows[id] = window;
            return id;
        }

        /**
         * @brief Retrieves the SDL window associated with the given ID.
         * @param id The ID of the window.
         * @return A pointer to the SDL window.
         */
        SDL_Window* GetWindow(uint32_t id)
        {
            return _windows[id];
        }

        /**
         * @brief Removes and destroys the SDL window associated with the given
         * ID.
         * @param id The ID of the window to remove.
         */
        void RemoveWindow(uint32_t id)
        {
            SDL_DestroyWindow(_windows[id]);
            _windows.erase(id);
        }

        /**
         * @brief Cleans up all SDL windows managed by the window manager.
         */
        void CleanUp()
        {
            for (auto& window : _windows)
            {
                SDL_DestroyWindow(window.second);
            }
            _windows.clear();
        }
    } // namespace Window
} // namespace RE