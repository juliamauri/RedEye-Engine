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

#ifndef RL_PROJECTS_CLASS
#define RL_PROJECTS_CLASS

#include <string>
#include <vector>

class RL_Projects
{
  private:
    enum class State : int
    {
        NONE = -1,
        NEW,
        LOAD
    } _state = State::NONE;

    struct Project
    {
        std::string name;
        std::string path;
    };

  public:
    bool Init();
    void CleanUp();

    void DrawGUI();

  private:
    void Load();
    void Save() const;

    bool GenerateProject(const char* name, const char* path);
    bool LoadProject(const char* path, std::string& outName);

  private:
    static constexpr const char* PROJECTS_FILE = "WriteDir/Projects.json";
    static constexpr const char* PROJECT_CONFIG_TEMPLATE = "template";
    static constexpr const char* IMGUI_CONFIG_TEMPLATE = "imgui.ini";
#ifdef _DEBUG
    static constexpr const char* ENGINE_EXECUTABLE = "Engine-d.exe";
#else
    static constexpr const char* ENGINE_EXECUTABLE = "Engine.exe";
#endif
    std::vector<Project> _projects;
};

#endif // RL_PROJECTS_CLASS