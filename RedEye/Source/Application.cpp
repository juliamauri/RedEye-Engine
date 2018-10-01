#include "Application.h"
#include "ModuleScene.h"
#include "Module.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "FileSystem.h"
#include "TimeManager.h"
#include "SystemInfo.h"
#include "RE_Math.h"
#include "OutputLog.h"
#include "Texture2DManager.h"
#include "ShaderManager.h"
#include "SDL2\include\SDL.h"
#include "ImGui\imgui.h"
#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"

using namespace std;

Application::Application(int argc, char* argv[])
{
	fs = new FileSystem();
	time = new TimeManager();
	sys_info = new SystemInfo();
	math = new RE_Math();
	log = new OutputLogHolder();

	modules.push_back(input = new ModuleInput("Input"));
	modules.push_back(window = new ModuleWindow("Window"));
	modules.push_back(scene = new ModuleScene("Scene"));
	modules.push_back(editor = new ModuleEditor("Editor"));
	modules.push_back(renderer3d = new ModuleRenderer3D("Renderer3D"));

	textures = new Texture2DManager("Images/");
	shaders = new ShaderManager("Shaders/");
}

Application::~Application()
{
	DEL(fs);
	DEL(time);
	DEL(sys_info);
	DEL(math);
	DEL(log);
	DEL(textures);
	DEL(shaders);

	for (list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		delete *it;
}

bool Application::Init()
{
	bool ret = true;

	// Initialize SDL without any subsystems
	if (SDL_Init(0) < 0)
	{
		LOG("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		SDL_version sdl_version;
		SDL_VERSION(&sdl_version);
		char tmp[8];
		sprintf_s(tmp, 8, "%u.%u.%u", (int)sdl_version.major, (int)sdl_version.minor, (int)sdl_version.patch);
		App->ReportSoftware("SDL", tmp, "https://www.libsdl.org/");
		sprintf_s(tmp, 8, "%u.%u.%u", IL_VERSION / 100, (IL_VERSION % 100) / 10, IL_VERSION % 10);
		App->ReportSoftware("DevIL", tmp, "http://openil.sourceforge.net/");

		if (fs->Init(argc, argv))
		{
			Config* config = fs->GetConfig();
			JSONNode* node = config->GetRootNode("App");
			app_name = node->PullString("Name", app_name.c_str());
			organization = node->PullString("Organization", organization.c_str());
			DEL(node);

			for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
				if ((*it)->IsActive() == true)
				{
					node = config->GetRootNode((*it)->GetName());
					ret = (*it)->Init(node);
					DEL(node);
				}

			sys_info->WhatAreWeRunningOn();
			math->Init();

			for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
				if ((*it)->IsActive() == true)
					ret = (*it)->Start(/* CONFIG */);
		}
	}

	return ret;
}

void Application::PrepareUpdate()
{
	time->UpdateDeltaTime();
}

int Application::Update()
{
	PrepareUpdate();

	int ret = UPDATE_CONTINUE;

	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->PreUpdate();

	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->Update();

	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->PostUpdate();

	FinishUpdate();

	if (want_to_quit && ret != UPDATE_ERROR)
		ret = UPDATE_STOP;

	return ret;
}

void Application::FinishUpdate()
{
	if (want_to_load)
		Load();

	if (want_to_save)
		Save();

	time->ManageFrameTimers();
}

bool Application::CleanUp()
{
	bool ret = true;

	fs->GetConfig()->Save();

	for (list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend() && ret; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->CleanUp();

	SDL_Quit();

	return ret;
}

bool Application::Load()
{
	bool ret = true;

	Config* config = fs->GetConfig();
	JSONNode* node = config->GetRootNode("App");
	app_name = node->PullString("Name", app_name.c_str());
	organization = node->PullString("Organization", organization.c_str());
	DEL(node);

	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
		if ((*it)->IsActive() == true)
		{
			node = config->GetRootNode((*it)->GetName());
			ret = (*it)->Load(node);
			DEL(node);
		}

	if (!ret) LOG("Error Loading Configuration to modules");

	want_to_load = false;

	return ret;
}

bool Application::Save()
{
	bool ret = true;

	Config* config = fs->GetConfig();
	JSONNode* node = config->GetRootNode("App");
	node->PushString("Name", app_name.c_str());
	node->PushString("Organization", organization.c_str());
	DEL(node);

	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
		if ((*it)->IsActive() == true)
		{
			node = config->GetRootNode((*it)->GetName());
			ret = (*it)->Save(node);
			DEL(node);
		}

	if (!ret) LOG("Error Save Configuration from modules");

	want_to_save = false;

	return ret;
}

void Application::DrawEditor()
{
	if (ImGui::BeginMenu("Options"))
	{
		//if (ImGui::MenuItem("Restore Defaults")) want_to_load_def = true;
		if (ImGui::MenuItem("Load")) want_to_load = true;
		if (ImGui::MenuItem("Save")) want_to_save = true;

		ImGui::EndMenu();
	}
	if (ImGui::CollapsingHeader("Application"))
	{
		static char holder[100];
		int size = sizeof(holder) / sizeof(char);

		sprintf_s(holder, size, "%s", app_name.c_str());
		if (ImGui::InputText("App Name", holder, size))
			app_name = holder;

		sprintf_s(holder, size, "%s", organization.c_str());
		if (ImGui::InputText("Organization", holder, size))
			organization = holder;

		time->DrawEditor();
	}

	for (list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		if ((*it)->IsActive() == true)
			(*it)->DrawEditor();

	if (ImGui::CollapsingHeader("Memory")) sys_info->MemoryDraw();
	if (ImGui::CollapsingHeader("Hardware")) sys_info->HardwareDraw();
	if (ImGui::CollapsingHeader("File System")) fs->DrawEditor();
}

void Application::Log(const char * text, const char* file)
{
	if(log != nullptr)
		log->Add(text, file);

	if(editor != nullptr && !modules.empty())
		editor->LogToEditorConsole();
}

void Application::ReportSoftware(const char * name, const char * version, const char * website)
{
	editor->AddSoftwareUsed(SoftwareInfo(name, version, website));
}

void Application::RecieveEvent(const Event* e)
{
	switch (e->GetType())
	{
	case REQUEST_DEFAULT_CONF: want_to_load_def = true; break;
	case REQUEST_LOAD: want_to_load = true; break;
	case REQUEST_SAVE: want_to_save = true; break;
	case REQUEST_QUIT: want_to_quit = true; break;
	}
}

const char * Application::GetName() const
{
	return app_name.c_str();
}

const char * Application::GetOrganization() const
{
	return organization.c_str();
}
