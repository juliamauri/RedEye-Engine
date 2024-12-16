#include "RL_Projects.h"

#include "RL_Application.h"
#include "RL_FileSystem.h"

#include <RapidJson/document.h>
#include <RapidJson/stringbuffer.h>
#include <RapidJson/writer.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <functional>
#include <string>

import FileSystem;
import Dialogs;
import GUIManager;

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
                    project_selected = RE::Dialogs::OpenDialog(L"reproject", RE::FileSystem::GetExecutableDirectory());
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
                        rapidjson::Document _template;
                        {
                            std::string _tempate_str = RE::FileSystem::Read(PROJECT_CONFIG_TEMPLATE);
                            _template.Parse(_tempate_str.c_str());
                        }
                        _template["Project"]["name"].SetString(_name.c_str(), _template.GetAllocator());
                        _template["Window"]["title"].SetString(_name.c_str(), _template.GetAllocator());

                        rapidjson::StringBuffer s_buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(s_buffer);
                        _template.Accept(writer);
                        s_buffer.Put('\0');

                        Project _p;
                        _p.name = _name;

                        _name += ".reproject";
                        project_selected += '\\';
                        RE::FileSystem::WriteOutside(project_selected.c_str(), _name.c_str(), s_buffer.GetString(),
                                                     s_buffer.GetSize());

                        std::string _imgui = RE::FileSystem::Read(IMGUI_CONFIG_TEMPLATE);
                        if (!_imgui.empty())
                            RE::FileSystem::WriteOutside(project_selected.c_str(), IMGUI_CONFIG_TEMPLATE,
                                                         _imgui.c_str(), _imgui.size());

                        _p.path = project_selected + _name;
                        _projects.push_back(_p);
                        Save();

                        _name = "Project Name";
                        project_selected.clear();
                    }

                    ImGui::Separator();
                }
                break;
                case RL_Projects::State::LOAD:
                {
                    rapidjson::Document _project;
                    std::string _config = RE::FileSystem::ReadOutside(project_selected.c_str());
                    if (!_config.empty())
                    {
                        _project.Parse(_config.c_str());
                        auto _project_settings = _project.FindMember("Project");
                        if (_project_settings != _project.MemberEnd())
                        {
                            Project _p;
                            _p.name = std::string(_project_settings->value["name"].GetString(),
                                                  _project_settings->value["name"].GetStringLength());
                            _p.path = project_selected;
                            _projects.push_back(_p);

                            Save();
                        }
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
    rapidjson::Document projects;
    {
        std::string data = RE::FileSystem::Read(PROJECTS_FILE);
        projects.Parse(data.c_str());
    }

    if (projects.IsArray())
    {
        auto _p = projects.GetArray();
        rapidjson::Value* iter = _p.begin();

        while (iter != _p.end())
        {
            Project _add;
            auto _v = iter->FindMember("name");
            if (_v != iter->MemberEnd())
                _add.name = _v->value.GetString();
            _v = iter->FindMember("path");
            if (_v != iter->MemberEnd())
                _add.path = _v->value.GetString();
            _projects.push_back(_add);

            iter++;
        }
    }
}

void RL_Projects::Save() const
{
    rapidjson::Document projects;
    projects.SetArray();

    for (Project _p : _projects)
    {
        rapidjson::Value _val;
        _val.SetObject();

        _val.AddMember("name", rapidjson::Value().SetString(_p.name.c_str(), projects.GetAllocator()),
                       projects.GetAllocator());
        _val.AddMember("path", rapidjson::Value().SetString(_p.path.c_str(), projects.GetAllocator()),
                       projects.GetAllocator());

        projects.PushBack(_val, projects.GetAllocator());
    }

    rapidjson::StringBuffer s_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s_buffer);
    projects.Accept(writer);
    s_buffer.Put('\0');

    RE::FileSystem::Write("", "projects.json", s_buffer.GetString(), s_buffer.GetSize());
}
