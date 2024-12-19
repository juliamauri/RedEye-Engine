#include "RL_Projects.h"

#include "RL_Application.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <functional>
#include <string>

import FileSystem;
import Dialogs;
import GUIManager;
import JSON;

bool RL_Projects::Init()
{
    if (RE::FileSystem::Exist(PROJECTS_FILE))
        Load();
    RE::GUI::AddMainWindow(std::bind(&RL_Projects::DrawGUI, this));
    return true;
}

void RL_Projects::CleanUp()
{
}

void RL_Projects::DrawGUI()
{
    static ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    // We demonstrate using the full viewport area or the work area (without
    // menu-bars, task-bars etc.) Based on your use case you may want one of the
    // other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGui::Begin("Projects", (bool*)0, flags))
    {

        bool _findpath = false;

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))
                {
                    _state = State::NEW;
                    _findpath = true;
                }
                if (ImGui::MenuItem("Load"))
                {
                    _state = State::LOAD;
                    _findpath = true;
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();

        static std::string project_selected;
        if (_findpath)
        {
            project_selected.clear();
            switch (_state)
            {
                case RL_Projects::State::NEW:
                    project_selected = RE::Dialogs::PickFolder(nullptr);
                    break;
                case RL_Projects::State::LOAD:
                    project_selected = RE::Dialogs::OpenDialog("reproject", RE::FileSystem::GetExecutableDirectory());
                    break;
            }
        }

        if (!project_selected.empty())
        {
            switch (_state)
            {
                case RL_Projects::State::NEW:
                {
                    static std::string _name = "New Project";
                    ImGui::InputText("Name", &_name);

                    ImGui::TextWrapped("Destination project: %s",
                                       (project_selected + "\\" + _name + ".reproject").c_str());

                    if (ImGui::Button("Generate"))
                    {
                        if (GenerateProject(_name.c_str(), project_selected.c_str()))
                        {
                            _projects.push_back({_name + ".reproject", project_selected + _name});
                            Save();
                        }

                        _name = "Project Name";
                        project_selected.clear();
                    }

                    ImGui::Separator();
                }
                break;
                case RL_Projects::State::LOAD:
                {
                    std::string _loadName;
                    if (LoadProject(project_selected.c_str(), _loadName))
                    {
                        _projects.push_back({_loadName, project_selected});
                        Save();
                    }

                    project_selected.clear();
                }
                break;
            }
        }

        std::vector<Project>::iterator to_erase = _projects.end();
        std::string _open_path;
        ImGui::Text("Projects");
        if (ImGui::BeginTable("Projects_table", 4, flags | ImGuiTableFlags_SizingFixedFit, ImVec2(0.0f, 0.0f)))
        {
            ImGui::TableSetupColumn("Open", ImGuiTableColumnFlags_NoHeaderLabel | ImGuiTableColumnFlags_NoSort |
                                                ImGuiTableColumnFlags_NoHeaderWidth | ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHeaderWidth |
                                                ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_NoHeaderLabel | ImGuiTableColumnFlags_NoSort |
                                                  ImGuiTableColumnFlags_NoHeaderWidth | ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupColumn("Path");

            ImGui::TableHeadersRow();
            unsigned int count = 0;
            for (auto iter = _projects.begin(); iter != _projects.end(); iter++, count++)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(("#OpenProject_" + iter->name + std::to_string(count)).c_str());
                if (ImGui::Button("Open"))
                    _open_path = iter->path;
                ImGui::PopID();

                ImGui::TableSetColumnIndex(1);
                ImGui::Text(iter->name.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::PushID(("#RemoveProject_" + iter->name + std::to_string(count)).c_str());
                if (ImGui::Button("Remove"))
                    to_erase = iter;
                ImGui::PopID();

                ImGui::TableSetColumnIndex(3);
                ImGui::Text(iter->path.c_str());
            }
        }
        ImGui::EndTable();
        if (to_erase != _projects.end())
        {
            _projects.erase(to_erase);
            Save();
        }

        if (!_open_path.empty())
        {
            std::string exec("start ");
            exec += '\"';
            exec += RE::FileSystem::GetExecutableDirectory();
            exec += '\"';
            exec += " ";
            exec += '\"';
            exec += ENGINE_EXECUTABLE;
            exec += '\"';
            exec += " ";
            exec += '\"';
            exec += _open_path;
            exec += '\"';
            system(exec.c_str());
        }
    }
    ImGui::End();
}

void RL_Projects::Load()
{
    uint32_t _projectsIn = 0;
    {
        std::string data = RE::FileSystem::Read(PROJECTS_FILE);
        if (data.empty())
            return;
        _projectsIn = RE::JSON::Parse(data);
    }
    _projects.clear();

    RE::JSON::PushSelected(_projectsIn);
    {
        using namespace RE::JSON::Value;
        Array::PullMode();
        while (Array::PullObject())
        {
            Project _add;
            _add.name = PullString("name", "Project Name");
            _add.path = PullString("path", "Project Path");
            _projects.push_back(_add);
        }
        Array::Disable();
    }
    RE::JSON::Destroy();
    RE::JSON::PopSelected();
}

void RL_Projects::Save() const
{
    uint32_t _projectsOut = RE::JSON::Create();
    std::string _buffer;

    RE::JSON::PushSelected(_projectsOut);
    {
        using namespace RE::JSON::Value;
        SetArray();
        Array::Enable(true);
        for (Project _p : _projects)
        {
            PushString(_p.name.c_str(), "name");
            PushString(_p.path.c_str(), "path");
            Array::PushObject();
        }
        Array::Disable();

        _buffer = RE::JSON::GetBuffer();
        RE::JSON::Destroy();
    }
    RE::JSON::PopSelected();

    RE::FileSystem::Write("", "projects.json", _buffer.c_str(), _buffer.size());
}

bool RL_Projects::GenerateProject(const char* _name, const char* _path)
{
    uint32_t _template = 0;
    std::string _buffer;
    {
        // Todo, sometimes the template is not found
        std::string _tempate_str = RE::FileSystem::Read(PROJECT_CONFIG_TEMPLATE);
        if (_tempate_str.empty())
            return false;
        _template = RE::JSON::Parse(_tempate_str);
    }
    RE::JSON::PushSelected(_template);
    {
        using namespace RE::JSON::Value;
        Push("Project");
        SetObject();
        PushString(_name, "name");
        Pop();
        Push("Window");
        SetObject();
        PushString(_name, "title");

        _buffer = RE::JSON::GetBuffer();
        RE::JSON::Destroy();
    }
    RE::JSON::PopSelected();

    std::string name(_name);
    name += ".reproject";
    std::string path(_path);
    path += '\\';
    RE::FileSystem::WriteOutside(path.c_str(), name.c_str(), _buffer.c_str(), _buffer.size());

    std::string _imgui = RE::FileSystem::Read(IMGUI_CONFIG_TEMPLATE);
    if (_imgui.empty())
        return false;
    RE::FileSystem::WriteOutside(path.c_str(), IMGUI_CONFIG_TEMPLATE, _imgui.c_str(), _imgui.size());

    return true;
}

bool RL_Projects::LoadProject(const char* path, std::string& outName)
{
    uint32_t _project = 0;
    std::string _buffer;
    {
        std::string _tempate_str = RE::FileSystem::ReadOutside(path);
        if (_tempate_str.empty())
            return false;
        _project = RE::JSON::Parse(_tempate_str);
    }
    RE::JSON::PushSelected(_project);
    {
        using namespace RE::JSON::Value;
        Push("Project");
        outName = PullString("name", "RedEye Project");
        RE::JSON::Destroy();
    }
    RE::JSON::PopSelected();
    return true;
}
