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

#ifndef JR_APPLICATION_CLASS
#define JR_APPLICATION_CLASS

class JR_Application
{
  public:
    bool Init(char* argv[]);

    void Update();

    void CleanUp();

    void RequestExit()
    {
        status = Status::WANT_EXIT;
    }

    void EventListener(union SDL_Event* event);

  public:
    static JR_Application* App;
    class JR_Input* input = nullptr;
    class JR_WindowAndRenderer* visual_magnament = nullptr;
    class RL_FileSystem* file_system = nullptr;
    class RL_Projects* projects_manager = nullptr;

  private:
    enum class Status
    {
        RUNNING,
        WANT_EXIT
    } status = Status::RUNNING;
};

#define APP JR_Application::App
#define JR_INPUT JR_Application::App->input
#define JR_VISUAL JR_Application::App->visual_magnament
#define RL_FS JR_Application::App->file_system
#define RL_PROJECTS JR_Application::App->projects_manager

#endif // !JR_APPLICATION_CLASS