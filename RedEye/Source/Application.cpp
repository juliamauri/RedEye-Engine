#include "Application.h"

#include "RE_Time.h"
#include "RE_Math.h"
#include "RE_ConsoleLog.h"
#include "RE_Hardware.h"
#include "RE_FileSystem.h"
#include "RE_Config.h"
#include "JSONNode.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"
#include "RE_ResourceManager.h"
#include "RE_ThumbnailManager.h"

#include "SDL2\include\SDL.h"
#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"
#include "Optick/include/optick.h"
#include <EAAssert/version.h>
#include <EAStdC/internal/Config.h>
#include <EAStdC/EASprintf.h>
#include <eathread/internal/config.h>

// Utility
RE_FileSystem* App::fs = nullptr;

// Modules
ModuleInput* App::input = nullptr;
ModuleWindow* App::window = nullptr;
ModuleScene* App::scene = nullptr;
ModuleEditor* App::editor = nullptr;
ModuleRenderer3D* App::renderer3d = nullptr;
ModuleAudio* App::audio = nullptr;
Config* Application::config = nullptr;

// Managers
RE_CameraManager App::cams;
RE_PrimitiveManager App::primitives;
RE_ResourceManager* Application::resources = nullptr;
RE_ThumbnailManager* Application::thumbnail = nullptr;

// Importers
RE_TextureImporter App::textures;
RE_ShaderImporter App::shaders;
RE_ModelImporter App::modelImporter;

Application* Application::instance = nullptr;
eastl::string Application::app_name = "RedEye Engine";
eastl::string Application::organization = "RedEye";
eastl::list<Module*> Application::modules;

Application::Application()
{
	modules.push_back(input = new ModuleInput());
	modules.push_back(window = new ModuleWindow());
	modules.push_back(scene = new ModuleScene());
	modules.push_back(editor = new ModuleEditor());
	modules.push_back(renderer3d = new ModuleRenderer3D());
	modules.push_back(audio = new ModuleAudio());

	fs = new RE_FileSystem();
	resources = new RE_ResourceManager();
	thumbnail = new RE_ThumbnailManager();

	instance = this;
}

Application::~Application()
{
	for (auto &mod : modules) delete mod;

	DEL(thumbnail);
	DEL(resources);
	DEL(fs);

	RE_Hardware::Clear();

	instance = nullptr;
}

bool Application::Init(int argc, char* argv[])
{
	bool ret = false;
	RE_LOG_SEPARATOR("Initializing Application");
	if (InitSDL())
	{
		if (InitFileSystem(argc, argv))
		{
			if (InitModules())
			{
				InitInternalSystems();
				if (!StartModules()) RE_LOG_ERROR("Application Init exits with ERROR");
				else ret = true;
			}
			else RE_LOG_ERROR("Application Init exits with ERROR");
		}
		else RE_LOG_ERROR("Application Init exits with ERROR");
	}
	else RE_LOG_ERROR("Application Init exits with ERROR");

	return ret;
}

void Application::MainLoop()
{
	RE_LOG_SEPARATOR("Entering Application's Main Loop");
	OPTICK_FRAME("MainThread RedEye");

	do {
		OPTICK_CATEGORY("Prepare Update - Application", Optick::Category::GameLogic);
		RE_Time::FrameDeltaTime();

		OPTICK_CATEGORY("PreUpdate - Application", Optick::Category::GameLogic);
		for (auto mod : modules) if (mod->IsActive()) mod->PreUpdate();

		OPTICK_CATEGORY("Update - Application", Optick::Category::GameLogic);
		for (auto mod : modules) if (mod->IsActive()) mod->Update();

		OPTICK_CATEGORY("PostUpdate - Application", Optick::Category::GameLogic);
		for (auto mod : modules) if (mod->IsActive()) mod->PostUpdate();

		OPTICK_CATEGORY("Load/Save module configuration - Application", Optick::Category::IO);
		if (want_to_load_config) LoadConfig();
		if (want_to_save_config) SaveConfig();

		OPTICK_CATEGORY("Finish Update - Application", Optick::Category::GameLogic);
		if (RE_Time::GetState() == GS_TICK)
		{
			RE_Time::PauseGameTimer();
			for (auto it = modules.rbegin(); it != modules.rend(); ++it)
				if ((*it)->IsActive()) (*it)->OnPause();
		}

		unsigned int extra_ms = RE_Time::FrameExtraMS();
		if (extra_ms > 0) extra_ms = fs->ReadAssetChanges(extra_ms);
		if (extra_ms > 0) extra_ms = audio->ReadBanksChanges(extra_ms);
		if (extra_ms > 0) SDL_Delay(extra_ms);

	} while (!want_to_quit);
}

void Application::CleanUp()
{
	Event::ClearQueue();
	if (save_config) config->Save();

	for (auto it = modules.rbegin(); it != modules.rend(); ++it)
		if ((*it)->IsActive()) (*it)->CleanUp();

	RE_Hardware::Clear();

	SDL_Quit();
}

void Application::DrawModuleEditorConfig()
{
	for (auto mod : modules) if (mod->IsActive()) mod->DrawEditor();
}

void Application::RecieveEvent(const Event& e)
{
	switch (e.type)
	{
	case PLAY: 
	{
		RE_Time::StartGameTimer();
		for (auto it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnPlay();

		break;
	}
	case PAUSE:
	{
		RE_Time::PauseGameTimer();
		for (auto it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnPause();

		break;
	}
	case TICK:
	{
		RE_Time::TickGameTimer();
		for (auto it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnPlay();

		break;
	}
	case STOP:
	{
		RE_Time::StopGameTimer();
		for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnStop();

		break;
	}
	case REQUEST_LOAD: want_to_load_config = true; break;
	case REQUEST_SAVE: want_to_save_config = true; break;
	case REQUEST_QUIT: want_to_quit = true; break;
	}
}

Application* Application::Ptr() { return instance; }
const char* Application::GetName() { return app_name.c_str(); }
const char* Application::GetOrganization() { return organization.c_str(); }

bool Application::InitSDL()
{
	bool ret = false;
	RE_LOG_SECONDARY("Initializing SDL without any subsystems");
	if (SDL_Init(0) == 0)
	{
		char tmp[8];
		SDL_version sdl_version;
		SDL_GetVersion(&sdl_version);
		EA::StdC::Snprintf(tmp, 8, "%u.%u.%u", sdl_version.major, sdl_version.minor, sdl_version.patch);
		RE_SOFT_NVS("SDL", tmp, "https://www.libsdl.org/");

		// exploit tmp to report un-initialized software
		EA::StdC::Snprintf(tmp, 8, "%i.%i.%i", EAASSERT_VERSION_MAJOR, EAASSERT_VERSION_MINOR, EAASSERT_VERSION_PATCH);
		RE_SOFT_NVS("EABase", EABASE_VERSION, "https://github.com/electronicarts/EABase");
		RE_SOFT_NVS("EASTL", EASTL_VERSION, "https://github.com/electronicarts/EASTL");
		RE_SOFT_NVS("EAStdC", EASTDC_VERSION, "https://github.com/electronicarts/EAStdC");
		RE_SOFT_NVS("EAAssert", tmp, "https://github.com/electronicarts/EAAssert");
		RE_SOFT_NVS("EAThread", EATHREAD_VERSION, "https://github.com/electronicarts/EAThread");
		RE_SOFT_NVS("Optick", "1.2.9", "https://optick.dev/");
		RE_SOFT_NS("MathGeoLib", "https://github.com/juj/MathGeoLib");
		RE_SOFT_N("gpudetect");

		ret = true;
	}
	else RE_LOG_ERROR("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());

	return ret;
}

bool Application::InitFileSystem(int argc, char* argv[])
{
	bool ret = false;
	if (fs->Init(argc, argv))
	{
		JSONNode* node = config->GetRootNode("App");
		app_name = node->PullString("Name", app_name.c_str());
		organization = node->PullString("Organization", organization.c_str());
		DEL(node);
		ret = true;
	}
	return ret;
}

void Application::InitInternalSystems()
{
	// Initialize Importers
	if (!textures.Init()) RE_LOG_WARNING("Won't be able to use/import textures");
	if (!shaders.Init()) RE_LOG_WARNING("Won't be able to use/import shaders");
	if (!modelImporter.Init()) RE_LOG_WARNING("Won't be able to use/import models");

	// Initialize Managers
	cams.Init();
	primitives.Init();
	thumbnail->Init();

	// Fetch Assets
	RE_ResourceManager::internalResources.Init();
	fs->ReadAssetChanges(0, true);
	audio->ReadBanksChanges();
}

bool Application::InitModules()
{
	RE_LOG_SEPARATOR("Initializing Modules");
	for (auto mod : modules)
		if (mod->IsActive())
			if (!mod->Init()) return false;

	return true;
}

bool Application::StartModules()
{
	RE_LOG_SEPARATOR("Starting Modules");
	for (auto mod : modules)
		if (mod->IsActive())
			if (!mod->Start()) return false;

	return true;
}

void Application::LoadConfig()
{
	want_to_load_config = false;

	JSONNode* node = config->GetRootNode("App");
	app_name = node->PullString("Name", app_name.c_str());
	organization = node->PullString("Organization", organization.c_str());
	DEL(node);

	for (auto mod : modules) if (mod->IsActive()) mod->Load();
}

void Application::SaveConfig()
{
	want_to_save_config = false;

	JSONNode* node = config->GetRootNode("App");
	node->PushString("Name", app_name.c_str());
	node->PushString("Organization", organization.c_str());
	DEL(node);

	for (auto mod : modules) if (mod->IsActive()) mod->Save();
}