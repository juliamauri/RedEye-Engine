#include "Application.h"

#include "RE_FileSystem.h"
#include "OutputLog.h"

#include "Module.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"

#include "RE_ResourceManager.h"
#include "RE_ThumbnailManager.h"
#include "SystemInfo.h"

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
RE_HandleErrors App::handlerrors;
RE_InternalResources App::internalResources;
RE_FileSystem* App::fs = nullptr;
OutputLogHolder* App::log = nullptr;

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

#ifdef _DEBUG
#include "SystemInfo.h"
SystemInfo* Application::sys_info = nullptr;
#endif // _DEBUG

eastl::string Application::app_name;
eastl::string Application::organization;
eastl::list<Module*> Application::modules;
GameState Application::state = GS_STOP;

Application* Application::instance = nullptr;

Application::Application()
{
	app_name = "RedEye Engine";
	organization = "RedEye";

	fs = new RE_FileSystem();
	log = new OutputLogHolder();

	modules.push_back(input = new ModuleInput());
	modules.push_back(window = new ModuleWindow());
	modules.push_back(scene = new ModuleScene());
	modules.push_back(editor = new ModuleEditor());
	modules.push_back(renderer3d = new ModuleRenderer3D());
	modules.push_back(audio = new ModuleAudio());

	resources = new RE_ResourceManager();
	thumbnail = new RE_ThumbnailManager();

#ifdef _DEBUG
	sys_info = new SystemInfo();
#endif // _DEBUG

	instance = this;
}

Application::~Application()
{
#ifdef _DEBUG
	DEL(sys_info);
#endif // _DEBUG

	DEL(thumbnail);
	DEL(resources);

	for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		delete *it;

	DEL(log);
	DEL(fs);

	instance = nullptr;
}

bool Application::Init(int argc, char* argv[])
{
	bool ret = true;

	RE_LOG("Initializing Application");
	RE_LOG_SECONDARY("Initializing SDL without any subsystems");

	if (SDL_Init(0) < 0)
	{
		RE_LOG_ERROR("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		char tmp[8];
		SDL_version sdl_version;
		SDL_GetVersion(&sdl_version);
		EA::StdC::Snprintf(tmp, 8, "%u.%u.%u", sdl_version.major, sdl_version.minor, sdl_version.patch);
		ReportSoftware("SDL", tmp, "https://www.libsdl.org/");

		ReportSoftware("EABase", EABASE_VERSION, "https://github.com/electronicarts/EABase");
		ReportSoftware("EASTL", EASTL_VERSION, "https://github.com/electronicarts/EASTL");
		ReportSoftware("EAStdC", EASTDC_VERSION, "https://github.com/electronicarts/EAStdC");
		EA::StdC::Snprintf(tmp, 8, "%i.%i.%i", EAASSERT_VERSION_MAJOR, EAASSERT_VERSION_MINOR, EAASSERT_VERSION_PATCH);
		ReportSoftware("EAAssert", tmp, "https://github.com/electronicarts/EAAssert");
		ReportSoftware("EAThread", EATHREAD_VERSION, "https://github.com/electronicarts/EAThread");

		ReportSoftware("Optick", "1.2.9", "https://optick.dev/");

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
				// Initialize Utility
				math.Init();
#ifdef _DEBUG
				sys_info->Init();
#endif // _DEBUG

				// Initialize Managers
				time.Init(60.f);
				cams.Init();
				primitives.Init();
				if (thumbnail) thumbnail->Init();

				// Initialize Importers
				if (!textures.Init()) RE_LOG_WARNING("Won't be able to use/import textures");
				if (!shaders.Init()) RE_LOG_WARNING("Won't be able to use/import shaders");
				if (!modelImporter.Init()) RE_LOG_WARNING("Won't be able to use/import models");

				internalResources.Init();

				fs->ReadAssetChanges(0, true);
				audio->ReadBanksChanges();

				// Start Modules
				RE_LOG_SEPARATOR("Starting Application");
				for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
				{
					if ((*it)->IsActive())
					{
						RE_LOG_SEPARATOR("Starting Module %s", (*it)->GetName());
						ret = (*it)->Start();
					}
				}
				resources->ThumbnailResources();
			}
		}
	}

	return ret;
}

void Application::PrepareUpdate()
{
	time.UpdateDeltaTime();
}

int Application::Update()
{
	OPTICK_CATEGORY("Update Application", Optick::Category::GameLogic);

	PrepareUpdate();

	int ret = UPDATE_CONTINUE;

	eastl::list<Module*>::iterator it;

	OPTICK_CATEGORY("PreUpdate Application", Optick::Category::GameLogic);
	for (it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->PreUpdate();

	OPTICK_CATEGORY("Update Application", Optick::Category::GameLogic);
	for (it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->Update();

	OPTICK_CATEGORY("PostUpdate Application", Optick::Category::GameLogic);
	for (it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->PostUpdate();

	FinishUpdate();

	if (want_to_quit && ret != UPDATE_ERROR)
		ret = UPDATE_STOP;

	return ret;
}

void Application::FinishUpdate()
{
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

	// Use extra miliseconds per frame
	unsigned int extra_ms = fs->ReadAssetChanges(time.ManageFrameTimers());
	if (extra_ms > 0) extra_ms = audio->ReadBanksChanges(extra_ms);
	if (extra_ms > 0) SDL_Delay(extra_ms);
}

bool Application::CleanUp()
{
	fs->GetConfig()->Save();

	bool ret = true;
	for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend() && ret; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->CleanUp();

	SDL_Quit();
	return ret;
}

Application* Application::Ptr()
{
	return instance;
}

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
			DEL(node); }

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

		time.DrawEditor();
	}

	for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		if ((*it)->IsActive())
			(*it)->DrawEditor();

	fs->DrawEditor();

#ifdef _DEBUG
	sys_info->DrawEditor();
#endif // _DEBUG
}

void Application::Log(int category, const char * text, const char* file)
{
	if (log)
	{
		log->Add(category, text, file);
		if (editor != nullptr && !modules.empty()) editor->LogToEditorConsole();
	}
}

void Application::ReportSoftware(const char * name, const char * version, const char * website)
{
	if (name) {
		if (version) {
			if (website) RE_LOG_SOFTWARE("%s v%s (%s)", name, version, website);
			else RE_LOG_SOFTWARE("%s v%s", name, version); }
		else {
			if (website) RE_LOG_SOFTWARE("%s (%s)", name, website);
			else RE_LOG_SOFTWARE(name); }
		if (editor) editor->AddSoftwareUsed(name, version, website); }
	else RE_LOG_ERROR("Reporting Software with invalid name");
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
	case REQUEST_DEFAULT_CONF: want_to_load_def = true; break;
	case REQUEST_LOAD: want_to_load = true; break;
	case REQUEST_SAVE: want_to_save = true; break;
	case REQUEST_QUIT: want_to_quit = true; break;
	}
}

const char * Application::GetName() { return app_name.c_str(); }
const char * Application::GetOrganization() { return organization.c_str(); }
GameState Application::GetState() { return state; }