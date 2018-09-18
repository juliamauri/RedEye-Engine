#include "Application.h"
#include "Module.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "FileSystem.h"
#include "TimeManager.h"
#include "SystemInfo.h"
#include "RE_Math.h"
#include "SDL2\include\SDL.h"
#include "ImGui\imgui.h"


using namespace std;

Application::Application(int argc, char* argv[])
{
	modules.push_back(input = new ModuleInput("Input"));
	modules.push_back(window = new ModuleWindow("Window"));
	modules.push_back(editor = new ModuleEditor("Editor"));
	modules.push_back(renderer3d = new ModuleRenderer3D("Renderer3D"));

	fs = new FileSystem();
	time = new TimeManager();
	sys_info = new SystemInfo();
	math = new RE_Math();
}

Application::~Application()
{
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

	if (fs->Init(argc, argv))
	{
		JSONNode* node = nullptr;
		Config* config = fs->GetConfig();

		for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
			if ((*it)->IsActive() == true)
			{
				node = config->GetRootNode((*it)->GetName());
				ret = (*it)->Init(node);
				delete node;
				node = nullptr;
			}
	}

	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->Start(/* CONFIG */);

	// Check hardware specs
	if (ret) sys_info->WhatAreWeRunningOn();

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
	time->ManageFrameTimers();
}

bool Application::CleanUp()
{
	bool ret = true;

	for (list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend() && ret; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->CleanUp();

	delete fs;
	delete time;
	delete sys_info;
	delete math;

	SDL_Quit();

	return ret;
}
void Application::DrawEditor()
{
	if (ImGui::CollapsingHeader("Application"))
	{
		static char holder[100];
		int size = sizeof(holder) / sizeof(char);

		sprintf_s(holder, size, "%s", app_name);
		if (ImGui::InputText("App Name", holder, size))
			window->SetTitle(app_name = holder);

		sprintf_s(holder, size, "%s", organization);
		if (ImGui::InputText("Organization", holder, size))
			app_name = holder;

		time->DrawEditor();
	}

	for (list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		if ((*it)->IsActive() == true)
			(*it)->DrawEditor();

	if (ImGui::CollapsingHeader("Memory")) sys_info->MemoryDraw();
	if (ImGui::CollapsingHeader("Hardware")) sys_info->HardwareDraw();
}

void Application::Log(const char * text)
{
	if(editor != nullptr && !modules.empty())
		editor->AddTextConsole(text);
}

void Application::RequestBrowser(const char* link) const
{
	ShellExecute(NULL, "open", link, NULL, NULL, SW_SHOWNORMAL);
}

void Application::RecieveEvent(const Event* e)
{
	if (e->GetType() == REQUEST_QUIT)
		want_to_quit = true;
}
