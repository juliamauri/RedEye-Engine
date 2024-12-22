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

#include "RL_WindowAndRenderer.h"

#include "RL_Application.h"

#include <SDL2/SDL.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_opengl3_loader.h>
#include <imgui_impl_sdl2.h>
#include <functional>

import EventSystem;
import WindowManager;
import GUIManager;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

bool JR_WindowAndRenderer::Init()
{
    if (!RE::Window::Init())
        return false;

    std::function<void(SDL_Event*)> listener = [this](SDL_Event* event) { this->EventListener(event); };
    RE::Event::SetWindowListener(listener);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    main_window = RE::Window::NewWindow("RedEye - Projects Explorer");
    SDL_Window* mainWindow = RE::Window::GetWindow(main_window);
    context = SDL_GL_CreateContext(mainWindow);
    SDL_GL_MakeCurrent(mainWindow, context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    return true;
}

void JR_WindowAndRenderer::PostUpdate()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    RE::GUI::Draw();

    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    static SDL_Window* window = RE::Window::GetWindow(main_window);
    SDL_GL_SwapWindow(window);
}

void JR_WindowAndRenderer::CleanUp()
{
    SDL_GL_DeleteContext(context);
}

SDL_Window* JR_WindowAndRenderer::GetMainWindow() const
{
    static SDL_Window* window = RE::Window::GetWindow(main_window);
    return window;
}

void JR_WindowAndRenderer::EventListener(SDL_Event* event)
{
    if (event->window.event == SDL_WINDOWEVENT_CLOSE && event->window.windowID == main_window)
        APP->RequestExit();
}