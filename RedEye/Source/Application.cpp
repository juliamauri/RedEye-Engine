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

#include "SDL2\include\SDL.h"
#include "ImGui\imgui.h"
#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"
#include "Optick/include/optick.h"

#include <EAAssert/version.h>
#include <EAStdC/internal/Config.h>
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

// Importers
RE_TextureImporter App::textures;
RE_ShaderImporter App::shaders;
RE_ModelImporter App::modelImporter;

// Managers
RE_TimeManager App::time;
RE_CameraManager App::cams;
RE_PrimitiveManager App::primitives;

RE_ResourceManager* Application::resources = nullptr;
RE_ThumbnailManager* Application::thumbnail = nullptr;

#ifdef _DEBUG
#include "SystemInfo.h"
SystemInfo* Application::sys_info = nullptr;
#endif // _DEBUG

eastl::string Application::app_name;
eastl::string Application::organization;
eastl::list<Module*> Application::modules;
GameState Application::state = GS_STOP;

Application::Application()
{
	app_name = "RedEye Engine";
	organization = "RedEye";

	fs = new RE_FileSystem();
	log = new OutputLogHolder();

	modules.push_back(input = new ModuleInput("Input"));
	modules.push_back(window = new ModuleWindow("Window"));
	modules.push_back(scene = new ModuleScene("Scene"));
	modules.push_back(editor = new ModuleEditor("Editor"));
	modules.push_back(renderer3d = new ModuleRenderer3D("Renderer3D"));
	modules.push_back(audio = new ModuleAudio("Wwise"));

	cams = new RE_CameraManager();
	textures = new RE_TextureImporter("Images/");
	shaders = new RE_ShaderImporter("Assets/Shaders/");
	primitives = new RE_PrimitiveManager();
	modelImporter = new RE_ModelImporter("Assets/Meshes/");
	resources = new RE_ResourceManager();
	internalResources = new RE_InternalResources();
	handlerrors = new RE_HandleErrors();
	glcache = new RE_GLCacheManager();
	fbomanager = new RE_FBOManager();
	thumbnail = new RE_ThumbnailManager();

#ifdef _DEBUG
	sys_info = new SystemInfo();
#endif // _DEBUG
}

Application::~Application()
{
	DEL(cams);
	DEL(textures);
	DEL(shaders);
	DEL(primitives);
	DEL(modelImporter);
	DEL(resources);
	DEL(internalResources);
	DEL(handlerrors);
	DEL(glcache);
	DEL(fbomanager);
	DEL(thumbnail);

	for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		delete *it;

	DEL(fs);
	DEL(time);
	DEL(math);
	DEL(log);

#ifdef _DEBUG
	DEL(sys_info);
#endif // _DEBUG
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

		ReportSoftware("EABase", EABASE_VERSION, "https://github.com/electronicarts/EABase");
		ReportSoftware("EASTL", EASTL_VERSION, "https://github.com/electronicarts/EASTL");
		ReportSoftware("EAStdC", EASTDC_VERSION, "https://github.com/electronicarts/EAStdC");
		sprintf_s(tmp, 8, "%u.%u.%u", (int)EAASSERT_VERSION_MAJOR, (int)EAASSERT_VERSION_MINOR, (int)EAASSERT_VERSION_PATCH);
		ReportSoftware("EAAssert", tmp, "https://github.com/electronicarts/EAAssert");
		ReportSoftware("EAThread", EATHREAD_VERSION, "https://github.com/electronicarts/EAThread");

		SDL_version sdl_version;
		SDL_VERSION(&sdl_version);
		sprintf_s(tmp, 8, "%u.%u.%u", (int)sdl_version.major, (int)sdl_version.minor, (int)sdl_version.patch);
		ReportSoftware("SDL", tmp, "https://www.libsdl.org/");

		ReportSoftware("Optick", "1.2.9", "https://optick.dev/");

		RE_LOG("Initializing File System");

		if (ret = fs->Init(argc, argv))
		{
			Config* config = fs->GetConfig();
			JSONNode* node = config->GetRootNode("App");
			app_name = node->PullString("Name", app_name.c_str());
			organization = node->PullString("Organization", organization.c_str());
			DEL(node);

			// Initiallize Updating Modules
			for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
				if ((*it)->IsActive() == true)
				{
					node = config->GetRootNode((*it)->GetName());

					if (node)
						RE_LOG("Initializing Module %s (%s)", (*it)->GetName(), node->GetDocumentPath());
					else
						RE_LOG("Initializing Module %s (empty)", (*it)->GetName());

					if (!(ret = (*it)->Init(node)))
						DEL(node);
				}

			if (ret)
			{
				// Initiallize Indenpendent Modules
				if (cams) cams->Init();
				if (time) time->Init(/*TODO get max fps from node*/);
				if (math) math->Init();
#ifdef _DEBUG
				if (sys_info) sys_info->Init();
#endif // _DEBUG

				// Initiallize Resource Managers
				if (textures && !textures->Init()) RE_LOG_WARNING("Won't be able to use textures");
				if (shaders && !shaders->Init()) RE_LOG_WARNING("Won't be able to use shaders");
				if (modelImporter && !modelImporter->Init())  RE_LOG_WARNING("Won't be able to import model");
				if (internalResources && !internalResources->Init())  RE_LOG_WARNING("Won't be able to load internal Resources");
				if (primitives && !primitives->Init("primitive"))  RE_LOG_WARNING("Won't be able to use primitives");
				if (thumbnail) thumbnail->Init();

				fs->ReadAssetChanges(0, true);
				audio->ReadBanksChanges();

				RE_LOG_SEPARATOR("Starting Application");

				for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
					if ((*it)->IsActive() == true)
					{
						RE_LOG_SEPARATOR("Starting Module %s", (*it)->GetName());
						ret = (*it)->Start();
					}
				resources->ThumbnailResources();
			}
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
		InstantEvent(PAUSE, this);
		ticking = false;
	}

	// Use extra miliseconds per frame
	unsigned int extra_ms = fs->ReadAssetChanges(time->ManageFrameTimers());
	if (extra_ms > 0) audio->ReadBanksChanges();
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

		time->DrawEditor();
	}

	for (eastl::list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		if ((*it)->IsActive() == true)
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

		time->StartGameTimer();
		state = GS_PLAY;
		ticking = false;
		break;
	}
	case PAUSE:
	{
		for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnPause();

		time->PauseGameTimer();
		state = GS_PAUSE;
		break;
	}
	case TICK:
	{
		for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnPlay();

		time->StartGameTimer();
		state = GS_PLAY;
		ticking = true;
		break;
	}
	case STOP:
	{
		for (eastl::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive()) (*it)->OnStop();

		time->StopGameTimer();
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