#include "Application.h"
#include "Module.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "FileSystem.h"
#include "SDL2\include\SDL.h"


using namespace std;

Application::Application(int argc, char* argv[])
{
	modules.push_back(input = new ModuleInput("Input"));
	modules.push_back(window = new ModuleWindow("Window"));

	fs = new FileSystem();
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



	/*/Config config;

	//if (config.LoadConfig()){
		Document* modules_config = config.GetConfig();
		if (!modules_config->IsObject())
		{
			LOG("Can't load config");
			return false;
		}

		for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
			if ((*it)->IsActive() == true)
			{
				if (modules_config->HasMember((*it)->GetName()))
					ret = (*it)->Init(config.GetMember((*it)->GetName()));
				else
				{
					LOG("Can't find config of %s module", (*it)->GetName());
					return false;
				}
			}
	}*/

	

	

	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->Start(/* CONFIG */);

	return ret;
}

void Application::PrepareUpdate()
{
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
}

bool Application::CleanUp()
{
	bool ret = true;

	for (list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend() && ret; ++it)
		if ((*it)->IsActive() == true)
			ret = (*it)->CleanUp();

	SDL_Quit();

	return ret;
}

void Application::RecieveEvent(const Event* e)
{
	if (e->GetType() == REQUEST_QUIT)
		want_to_quit = true;
}