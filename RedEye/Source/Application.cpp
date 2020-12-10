#include "Application.h"

#include "RE_Time.h"
#include "RE_ConsoleLog.h"
#include "RE_Hardware.h"
#include "RE_FileSystem.h"
#include "RE_Config.h"
#include "RE_Json.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"
#include "RE_ModelImporter.h"
#include "RE_ShaderImporter.h"
#include "RE_TextureImporter.h"
#include "RE_ResourceManager.h"
#include "RE_ThumbnailManager.h"
#include "RE_PrimitiveManager.h"

#include "SDL2\include\SDL.h"
#include "Optick/include/optick.h"
#include <EAAssert/version.h>
#include <EAStdC/internal/Config.h>
#include <EAStdC/EASprintf.h>
#include <eathread/internal/config.h>

// Modules
ModuleInput* App::input = nullptr;
ModuleWindow* App::window = nullptr;
ModuleScene* App::scene = nullptr;
ModuleEditor* App::editor = nullptr;
ModuleRenderer3D* App::renderer3d = nullptr;
ModuleAudio* App::audio = nullptr;

App* App::instance = nullptr;
eastl::string App::app_name = "RedEye Engine";
eastl::string App::organization = "RedEye";
eastl::list<Module*> App::modules;

Application::Application()
{
	modules.push_back(input = new ModuleInput());
	modules.push_back(window = new ModuleWindow());
	modules.push_back(scene = new ModuleScene());
	modules.push_back(editor = new ModuleEditor());
	modules.push_back(renderer3d = new ModuleRenderer3D());
	modules.push_back(audio = new ModuleAudio());

	instance = this;
}

Application::~Application()
{
	for (auto &mod : modules) delete mod;

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
	OPTICK_FRAME("MainThread RedEye");
	RE_LOG_SEPARATOR("Entering Application's Main Loop - %.3f", RE_Time::FrameDeltaTime());

	do {
		OPTICK_CATEGORY("Application Update", Optick::Category::GameLogic);
		RE_Time::FrameDeltaTime();
		for (auto mod : modules) if (mod->IsActive()) mod->PreUpdate();
		for (auto mod : modules) if (mod->IsActive()) mod->Update();
		for (auto mod : modules) if (mod->IsActive()) mod->PostUpdate();

		if (load_config) LoadConfig();
		if (save_config) SaveConfig();
		if (RE_Time::GetState() == GS_TICK)
		{
			RE_Time::PauseGameTimer();
			for (auto it = modules.rbegin(); it != modules.rend(); ++it)
				if ((*it)->IsActive()) (*it)->OnPause();
		}

		unsigned int extra_ms = RE_Time::FrameExtraMS();
		if (extra_ms > 0) extra_ms = RE_FileSystem::ReadAssetChanges(extra_ms);
		if (extra_ms > 0) extra_ms = audio->ReadBanksChanges(extra_ms);
		if (extra_ms > 0)
		{
			OPTICK_CATEGORY("Application Delay extra ms", Optick::Category::WaitEmpty);
			RE_Time::Delay(extra_ms);
		}

	} while (!quit);
}

void Application::CleanUp()
{
	Event::ClearQueue();
	if (RE_FileSystem::config != nullptr && save_config) RE_FileSystem::config->Save();

	for (auto it = modules.rbegin(); it != modules.rend(); ++it)
		if ((*it)->IsActive()) (*it)->CleanUp();

	RE_Hardware::Clear();
	RE_CameraManager::Clear();
	RE_PrimitiveManager::Clear();
	RE_ThumbnailManager::Clear();
	RE_ResourceManager::Clear();

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
		for (auto it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnStop();

		break;
	}
	case REQUEST_LOAD: load_config = true; break;
	case REQUEST_SAVE: save_config = true; break;
	case REQUEST_QUIT: quit = true; break;
	case RESOURCE_CHANGED: RE_ResourceManager::ResourceHasChanged(e.data1.AsCharP()); break;
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
	if (RE_FileSystem::Init(argc, argv))
	{
		RE_Json* node = RE_FileSystem::config->GetRootNode("App");
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
	RE_ModelImporter::Init();
	RE_ShaderImporter::Init();
	if (!RE_TextureImporter::Init()) RE_LOG_WARNING("Won't be able to use/import textures");
	
	// Initialize Managers
	RE_CameraManager::Init();
	RE_PrimitiveManager::Init();
	RE_ThumbnailManager::Init();
	RE_ResourceManager::Init();

	// Fetch Assets
	RE_FileSystem::ReadAssetChanges(0, true);
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
	OPTICK_CATEGORY("Load module configuration - Application", Optick::Category::IO);
	load_config = false;

	RE_Json* node = RE_FileSystem::config->GetRootNode("App");
	app_name = node->PullString("Name", app_name.c_str());
	organization = node->PullString("Organization", organization.c_str());
	DEL(node);

	for (auto mod : modules) if (mod->IsActive()) mod->Load();
}

void Application::SaveConfig()
{
	OPTICK_CATEGORY("Save module configuration - Application", Optick::Category::IO);
	save_config = false;

	RE_Json* node = RE_FileSystem::config->GetRootNode("App");
	node->PushString("Name", app_name.c_str());
	node->PushString("Organization", organization.c_str());
	DEL(node);

	for (auto mod : modules) if (mod->IsActive()) mod->Save();
}