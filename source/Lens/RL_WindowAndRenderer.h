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

#ifndef JR_WINDOWRENDERER_CLASS
#define JR_WINDOWRENDERER_CLASS

#include <SDL2/SDL_render.h>

class JR_WindowAndRenderer
{
  public:
    bool Init();

    void PostUpdate();

    void CleanUp();

    SDL_Window* GetMainWindow() const;
    SDL_GLContext GetContext() const
    {
        return context;
    }

  private:
    void EventListener(union SDL_Event* event);

  private:
    uint32_t main_window = 0;
    SDL_GLContext context = nullptr;
};

#endif // !JR_WINDOWRENDERER_CLASS