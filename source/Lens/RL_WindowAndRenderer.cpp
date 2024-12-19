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
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) == 0)
    {
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
        SDL_GL_SetSwapInterval(1); // Enable vsynci

        return true;
    }

    return false;
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
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
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