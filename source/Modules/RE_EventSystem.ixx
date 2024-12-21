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
#include <functional>
#include <vector>
#include <algorithm>
#include <typeinfo>

export module EventSystem;

std::function<void(SDL_Event*)> _systemListener;
std::function<void(SDL_Event*)> _inputListener;
std::function<void(SDL_Event*)> _windowListener;
std::vector<unsigned int> _customEvents;
std::vector<std::function<void(SDL_Event*)>> _customlisteners;

constexpr const uint32_t EVENT_NULL = -1;

export namespace RE
{
    namespace Event
    {
        /**
         * @brief Initializes the event system with a system listener.
         * @param systemlistener The system listener function to handle events.
         * @return True if initialization was successful, false otherwise.
         */
        bool Init(std::function<void(SDL_Event*)> systemlistener)
        {
            _systemListener = systemlistener;
            if (SDL_InitSubSystem(SDL_INIT_EVENTS))
                return false;
            return true;
        }

        /**
         * @brief Sets the input listener function.
         * @param listener The input listener function to handle input events.
         */
        void SetInputListener(std::function<void(SDL_Event*)> listener)
        {
            _inputListener = listener;
        }

        /**
         * @brief Sets the window listener function.
         * @param listener The window listener function to handle window events.
         */
        void SetWindowListener(std::function<void(SDL_Event*)> listener)
        {
            _windowListener = listener;
        }

        /**
         * @brief Adds a custom event type.
         * @param event The custom event type to add.
         */
        void AddCustomEvent(uint32_t& event)
        {
            event = SDL_RegisterEvents(1);
            if (event != EVENT_NULL)
                _customEvents.push_back(event);
        }

        /**
         * @brief Adds a custom event listener function.
         * @param listener The custom event listener function to handle custom
         * events.
         */
        void AddCustomListener(std::function<void(SDL_Event*)> listener)
        {
            _customlisteners.push_back(listener);
        }

        /**
         * @brief Pumps and processes SDL events.
         */
        void PumpEvents()
        {
            SDL_PumpEvents();

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                _systemListener(&event);
                _inputListener(&event);
                if (event.type == SDL_WINDOWEVENT)
                    _windowListener(&event);

                if (_customEvents.empty() || _customlisteners.empty())
                    continue;
                auto isCustom = std::find(_customEvents.begin(), _customEvents.end(), event.type);
                if (isCustom != _customEvents.end())
                {
                    for (auto& listener : _customlisteners)
                    {
                        listener(&event);
                    }
                }
            }
        }
    } // namespace Event
} // namespace RE