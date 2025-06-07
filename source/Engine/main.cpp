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

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <GL/gl.h>
#include <cstdlib>

#include "RE_Application.h"

int main(int argc, char* argv[])
{
    if (!Application::Init(argc, argv))
        return EXIT_FAILURE;

    Application::MainLoop();
    Application::CleanUp();
    return EXIT_SUCCESS;
}
