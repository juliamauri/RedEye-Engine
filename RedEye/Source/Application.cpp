#include "Application.h"

#include "RE_FileSystem.h"
#include "RE_LogManager.h"

#include "Module.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"

#include "RE_LogManager.h"
#include "RE_ResourceManager.h"
#include "RE_ThumbnailManager.h"
#include "RE_Hardware.h"

#include "SDL2\include\SDL.h"
#include "ImGui\imgui.h"
#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"
#include "Optick/include/optick.h"
#include <EAAssert/version.h>
#include <EAStdC/internal/Config.h>
#include <EAStdC/EASprintf.h>
#include <eathread/internal/config.h>

// Utility
RE_Math App::math;
RE_FileSystem* App::fs = nullptr;
RE_Hardware* Application::hardware = nullptr;

// Modules
ModuleInput* App::input = nullptr;
ModuleWindow* App::window = nullptr;
ModuleScene* App::scene = nullptr;
ModuleEditor* App::editor = nullptr;
ModuleRenderer3D* App::renderer3d = nullptr;
ModuleAudio* App::audio = nullptr;

// Managers
RE_TimeManager App::time;
RE_CameraManager App::cams;
RE_PrimitiveManager App::primitives;
RE_ResourceManager* Application::resources = nullptr;
RE_ThumbnailManager* Application::thumbnail = nullptr;

// Importers
RE_TextureImporter App::textures;
RE_ShaderImporter App::shaders;
RE_ModelImporter App::modelImporter;

eastl::string Application::app_name;
eastl::string Application::organization;
eastl::list<Module*> Application::modules;
GameState Application::state = GS_STOP;

Application* Application::instance = nullptr;

Application::Application()
{
	instance = this;
	app_name = "RedEye Engine";
	organization = "RedEye";

	fs = new RE_FileSystem();

	modules.push_back(input = new ModuleInput());
	modules.push_back(window = new ModuleWindow());
	modules.push_back(scene = new ModuleScene());
	modules.push_back(editor = new ModuleEditor());
	modules.push_back(renderer3d = new ModuleRenderer3D());
	modules.push_back(audio = new ModuleAudio());

	resources = new RE_ResourceManager();
	thumbnail = new RE_ThumbnailManager();
	hardware = new RE_Hardware();
}

Application::~Application()
{
	DEL(hardware);
	DEL(thumbnail);
	DEL(resources);

	for (auto &mod : modules)
		delete mod;

	DEL(fs);

	instance = nullptr;
}

bool Application::Init(int argc, char* argv[])
{
	bool ret = true;

	RE_LOG_SEPARATOR("Initializing Application");

	char tmp[8];
	EA::StdC::Snprintf(tmp, 8, "%i.%i.%i", EAASSERT_VERSION_MAJOR, EAASSERT_VERSION_MINOR, EAASSERT_VERSION_PATCH);
	RE_SOFT_NVS("EABase", EABASE_VERSION, "https://github.com/electronicarts/EABase");
	RE_SOFT_NVS("EASTL", EASTL_VERSION, "https://github.com/electronicarts/EASTL");
	RE_SOFT_NVS("EAStdC", EASTDC_VERSION, "https://github.com/electronicarts/EAStdC");
	RE_SOFT_NVS("EAAssert", tmp, "https://github.com/electronicarts/EAAssert");
	RE_SOFT_NVS("EAThread", EATHREAD_VERSION, "https://github.com/electronicarts/EAThread");
	RE_SOFT_NVS("Optick", "1.2.9", "https://optick.dev/");

	RE_LOG_SECONDARY("Initializing SDL without any subsystems");

	if (SDL_Init(0) < 0)
	{
		RE_LOG_ERROR("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		SDL_version sdl_version;
		SDL_GetVersion(&sdl_version);
		EA::StdC::Snprintf(tmp, 8, "%u.%u.%u", sdl_version.major, sdl_version.minor, sdl_version.patch);
		RE_SOFT_NVS("SDL", tmp, "https://www.libsdl.org/");

		RE_LOG("Initializing File System");

		if (ret = fs->Init(argc, argv))
		{
			Config* config = fs->GetConfig();
			JSONNode* node = config->GetRootNode("App");
			app_name = node->PullString("Name", app_name.c_str());
			organization = node->PullString("Organization", organization.c_str());
			DEL(node);

			// Initialize Modules
			for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
			{
				if ((*it)->IsActive())
				{
					const char* name = (*it)->GetName();
					RE_LOG("Initializing Module %s (%s)", name, (node = config->GetRootNode(name)) ? node->GetDocumentPath() : "empty json");
					if (!(ret = (*it)->Init(node))) DEL(node);
				}
			}

			if (ret)
			{
				// Initialize Managers
				time.Init(60.f);
				cams.Init();
				primitives.Init();
				if (thumbnail) thumbnail->Init();

				// Initialize Importers
				if (!textures.Init()) RE_LOG_WARNING("Won't be able to use/import textures");
				if (!shaders.Init()) RE_LOG_WARNING("Won't be able to use/import shaders");
				if (!modelImporter.Init()) RE_LOG_WARNING("Won't be able to use/import models");

				// Initialize Utility
				math.Init();
				hardware->Init();

				// Load resources
				resources->Init(); // internal resources
				fs->ReadAssetChanges(0, true);
				audio->ReadBanksChanges();

				// Start Modules
				RE_LOG_SEPARATOR("Starting Application");
				for (auto mod : modules)
				{
					if (mod->IsActive())
					{
						RE_LOG_SEPARATOR("Starting Module %s", mod->GetName());
						ret = mod->Start();
					}
				}

				resources->ThumbnailResources();
			}
		}
	}

	if (ret) RE_LOG_SEPARATOR("Entering Application's Main Loop");
	else RE_LOG_ERROR("Application Init exits with ERROR");

	return ret;
}

void Application::MainLoop()
{
	do
	{
		OPTICK_FRAME("MainThread RedEye");
		OPTICK_CATEGORY("Prepare Update - Application", Optick::Category::GameLogic);

		time.UpdateDeltaTime();

		OPTICK_CATEGORY("PreUpdate - Application", Optick::Category::GameLogic);
		for (auto mod : modules) if (mod->IsActive()) mod->PreUpdate();

		OPTICK_CATEGORY("Update - Application", Optick::Category::GameLogic);
		for (auto mod : modules) if (mod->IsActive()) mod->Update();

		OPTICK_CATEGORY("PostUpdate - Application", Optick::Category::GameLogic);
		for (auto mod : modules) if (mod->IsActive()) mod->PostUpdate();

		OPTICK_CATEGORY("Finish Update - Application", Optick::Category::GameLogic);
		if (want_to_load) Load();
		if (want_to_save) Save();
		if (ticking)
		{
			for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
				if ((*it)->IsActive()) (*it)->OnPause();

			time.PauseGameTimer();
			state = GS_PAUSE;
			ticking = false;
		}

		OPTICK_CATEGORY("Read Asset Changes - File system", Optick::Category::IO);
		// Use extra miliseconds per frame
		unsigned int extra_ms = fs->ReadAssetChanges(time.ManageFrameTimers());
		if (extra_ms > 0) extra_ms = audio->ReadBanksChanges(extra_ms);
		if (extra_ms > 0) SDL_Delay(extra_ms);

	} while (!want_to_quit);
}

void Application::CleanUp()
{
	Event::ClearQueue();
	fs->GetConfig()->Save();

	for (auto it = modules.rbegin(); it != modules.rend(); ++it)
		if ((*it)->IsActive()) (*it)->CleanUp();

	SDL_Quit();
}

void Application::DrawEditor()
{
	if (ImGui::BeginMenu("Options"))
	{
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

		time.DrawEditor();
	}

	for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		if ((*it)->IsActive())
			(*it)->DrawEditor();

	fs->DrawEditor();
	hardware->DrawEditor();
}

void Application::RecieveEvent(const Event& e)
{
	switch (e.type)
	{
	case PLAY: 
	{
		for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnPlay();

		time.StartGameTimer();
		state = GS_PLAY;
		ticking = false;
		break;
	}
	case PAUSE:
	{
		for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnPause();

		time.PauseGameTimer();
		state = GS_PAUSE;
		break;
	}
	case TICK:
	{
		for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnPlay();

		time.StartGameTimer();
		state = GS_PLAY;
		ticking = true;
		break;
	}
	case STOP:
	{
		for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnStop();

		time.StopGameTimer();
		state = GS_STOP;
		break;
	}
	case REQUEST_LOAD: want_to_load = true; break;
	case REQUEST_SAVE: want_to_save = true; break;
	case REQUEST_QUIT: want_to_quit = true; break;
	}
}

Application* Application::Ptr() { return instance; }
const char* Application::GetName() { return app_name.c_str(); }
const char* Application::GetOrganization() { return organization.c_str(); }
GameState Application::GetState() { return state; }

bool Application::Load()
{
	want_to_load = false;
	Config* config = fs->GetConfig();
	JSONNode* node = config->GetRootNode("App");
	app_name = node->PullString("Name", app_name.c_str());
	organization = node->PullString("Organization", organization.c_str());
	DEL(node);

	bool ret = true;
	for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
		if ((*it)->IsActive() == true) {
			node = config->GetRootNode((*it)->GetName());
			ret = (*it)->Load(node);
			DEL(node);
		}

	if (!ret) RE_LOG("Error Loading Configuration to modules");

	return ret;
}

bool Application::Save()
{
	want_to_save = false;
	Config* config = fs->GetConfig();
	JSONNode* node = config->GetRootNode("App");
	node->PushString("Name", app_name.c_str());
	node->PushString("Organization", organization.c_str());
	DEL(node);

	bool ret = true;
	for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
		if ((*it)->IsActive() == true)
		{
			node = config->GetRootNode((*it)->GetName());
			ret = (*it)->Save(node);
			DEL(node);
		}

	if (!ret) RE_LOG_ERROR("Error Save Configuration from modules");

	return ret;
}