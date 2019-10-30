#include "Application.h"

#include "Module.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"

#include "RE_Math.h"
#include "FileSystem.h"
#include "RE_PrimitiveManager.h"
#include "ResourceManager.h"

#include "ShaderManager.h"
#include "RE_TextureImporter.h"
#include "RE_ModelImporter.h"
#include "RE_InternalResources.h"

#include "TimeManager.h"
#include "SystemInfo.h"
#include "OutputLog.h"
#include "RE_HandleErrors.h"

#include "SDL2\include\SDL.h"
#include "ImGui\imgui.h"
#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"
#include "Optick/include/optick.h"

using namespace std;

Application::Application()
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

	textures = new RE_TextureImporter("Images/");
	shaders = new ShaderManager("Assets/Shaders/");
	primitives = new RE_PrimitiveManager();
	modelImporter = new RE_ModelImporter("Assets/Meshes/");
	resources = new ResourceManager();
	internalResources = new RE_InternalResources();
	handlerrors = new RE_HandleErrors();
}

Application::~Application()
{
	DEL(textures);
	DEL(shaders);
	DEL(primitives);
	DEL(modelImporter);
	DEL(resources);
	DEL(internalResources);
	DEL(handlerrors);

	for (list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		delete *it;

	DEL(fs);
	DEL(time);
	DEL(sys_info);
	DEL(math);
	DEL(log);
}

bool Application::Init(int argc, char* argv[])
{
	bool ret = true;

	LOG("Initializing Application");
	LOG_SECONDARY("Initializing SDL without any subsystems");

	if (SDL_Init(0) < 0)
	{
		LOG_ERROR("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		SDL_version sdl_version;
		SDL_VERSION(&sdl_version);
		char tmp[8];
		sprintf_s(tmp, 8, "%u.%u.%u", (int)sdl_version.major, (int)sdl_version.minor, (int)sdl_version.patch);
		App->ReportSoftware("SDL", tmp, "https://www.libsdl.org/");

		App->ReportSoftware("Optick", "1.2.9", "https://optick.dev/");

		LOG("Initializing File System");

		if (ret = fs->Init(argc, argv))
		{
			Config* config = fs->GetConfig();
			JSONNode* node = config->GetRootNode("App");
			app_name = node->PullString("Name", app_name.c_str());
			organization = node->PullString("Organization", organization.c_str());
			DEL(node);

			// Initiallize Updating Modules
			for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
				if ((*it)->IsActive() == true)
				{
					node = config->GetRootNode((*it)->GetName());

					if (node)
						LOG("Initializing Module %s (%s)", (*it)->GetName(), node->GetDocumentPath());
					else
						LOG("Initializing Module %s (empty)", (*it)->GetName());

					if (!(ret = (*it)->Init(node)))
						DEL(node);
				}

			if (ret)
			{
				// Initiallize Indenpendent Modules
				if (time) time->Init(/*TODO get max fps from node*/);
				if (sys_info) sys_info->Init();
				if (math) math->Init();

				// Initiallize Resource Managers
				if (textures && !textures->Init()) LOG_WARNING("Won't be able to use textures");
				if (shaders && !shaders->Init()) LOG_WARNING("Won't be able to use shaders");
				if (modelImporter && !modelImporter->Init())  LOG_WARNING("Won't be able to import model");
				if (internalResources && !internalResources->Init())  LOG_WARNING("Won't be able to load internal Resources");
				if (primitives && !primitives->Init("primitive"))  LOG_WARNING("Won't be able to use primitives");
				

				LOG_SEPARATOR("Starting Application");

				for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
					if ((*it)->IsActive() == true)
					{
						LOG_SEPARATOR("Starting Module %s", (*it)->GetName());
						ret = (*it)->Start();
					}
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

	OPTICK_CATEGORY("PreUpdate Application", Optick::Category::GameLogic);
	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->PreUpdate();

	OPTICK_CATEGORY("Update Application", Optick::Category::GameLogic);
	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->Update();

	OPTICK_CATEGORY("PostUpdate Application", Optick::Category::GameLogic);
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

	if (!ret) LOG_ERROR("Error Save Configuration from modules");

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

void Application::Log(int category, const char * text, const char* file)
{
	if (log != nullptr)
	{
		log->Add(category, text, file);

		if (editor != nullptr && !modules.empty())
			editor->LogToEditorConsole();
	}
}

void Application::ReportSoftware(const char * name, const char * version, const char * website)
{
	if (name != nullptr)
	{
		if (version != nullptr)
		{
			if (website != nullptr)
				LOG_SOFTWARE("%s v%s (%s)", name, version, website);
			else
				LOG_SOFTWARE("%s v%s", name, version);
		}
		else
		{
			if (website != nullptr)
				LOG_SOFTWARE("%s (%s)", name, website);
			else
				LOG_SOFTWARE(name);
		}

		if (editor != nullptr)
			editor->AddSoftwareUsed(name, version, website);
	}
	else
	{
		LOG_ERROR("Reporting Software with invalid name");
	}
}

void Application::RecieveEvent(const Event& e)
{
	switch (e.GetType())
	{
	case PLAY: 
	{
		state = GS_PLAY;
		time->StartGameTimer();

		for (list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive() == true)
				(*it)->OnPlay();
		
		break;
	}
	case PAUSE:
	{
		state = GS_PAUSE;
		time->PauseGameTimer();

		for (list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive() == true)
				(*it)->OnPause();

		break;
	}
	case STOP:
	{
		state = GS_STOP;
		time->StopGameTimer();

		for (list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
			if ((*it)->IsActive() == true)
				(*it)->OnStop();

		// load saved scene
		break;
	}
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

GameState Application::GetState() const
{
	return state;
}

void Application::ScenePlay()
{
	input->AddEvent(Event(PLAY, this));
}

void Application::ScenePause()
{
	input->AddEvent(Event(PAUSE, this));
}

void Application::SceneStop()
{
	input->AddEvent(Event(STOP, this));
}
