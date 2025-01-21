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
#include <GL/glew.h>
#include <GL/gl.h>

export module OpenGL;

bool glew_initialized = false;

export namespace RE
{
    namespace OpenGL
    {
        struct Context
        {
            SDL_GLContext context;

            bool Create(SDL_Window* window)
            {
                context = SDL_GL_CreateContext(window);

                if (glew_initialized || context == NULL)
                    return context != NULL;

                if (glewInit() != GLEW_OK)
                {
                    Delete();
                    return false;
                }

                return glew_initialized = true;
            }

            void Delete()
            {
                SDL_GL_DeleteContext(context);
            }

            void RenderTriangle()
            {
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                glBegin(GL_TRIANGLES);
                glColor3f(1.0f, 0.0f, 0.0f);
                glVertex2f(0.0f, 0.5f);
                glVertex2f(-0.5f, -0.5f);
                glVertex2f(0.5f, -0.5f);
                glEnd();
            }
        };

    } // namespace OpenGL
} // namespace RE