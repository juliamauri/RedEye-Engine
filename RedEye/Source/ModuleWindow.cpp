#include "ModuleWindow.h"

#include "Application.h"
#include "SDL2\include\SDL.h"

ModuleWindow::ModuleWindow(const char* name, bool start_enabled) : Module(name, start_enabled),
window(nullptr), screen_surface(nullptr), flags(0)
{}

// Destructor
ModuleWindow::~ModuleWindow()
{}

// Called before render is available
bool ModuleWindow::Init(rapidjson::Value::ConstMemberIterator config_module)
{
	LOG("Init SDL window & surface");
	bool ret = true;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		//Use OpenGL 2.1
		/*SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;*/

		const char* title = "";
		uint screen_width;
		uint screen_height;
		bool fullscreen;
		bool resizable;
		bool borderless;
		bool fullscreen_desktop;

		if (config_module->value.HasMember("title"))
			title = config_module->value["title"].GetString();
		if(config_module->value.HasMember("screen_width"))
			screen_width = config_module->value["screen_width"].GetInt();
		if(config_module->value.HasMember("screen_height"))
			screen_height = config_module->value["screen_height"].GetInt();
		if (config_module->value.HasMember("fullscreen"))
			fullscreen = config_module->value["fullscreen"].GetBool();
		if (config_module->value.HasMember("resizable"))
			resizable = config_module->value["resizable"].GetBool();
		if (config_module->value.HasMember("borderless"))
			borderless = config_module->value["borderless"].GetBool();
		if (config_module->value.HasMember("fullscreen_desktop"))
			fullscreen_desktop = config_module->value["fullscreen_desktop"].GetBool();

		if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
		if (resizable) flags |= SDL_WINDOW_RESIZABLE;
		if (borderless) flags |= SDL_WINDOW_BORDERLESS;
		if (fullscreen_desktop) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		//Create window
		window = SDL_CreateWindow(
			title, // Title
			SDL_WINDOWPOS_CENTERED, // x position
			SDL_WINDOWPOS_CENTERED, // y position
			screen_width, // width
			screen_height, // height
			flags); // flags

		if (!window)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			screen_surface = SDL_GetWindowSurface(window);
		}
	}

	return ret;
}

update_status ModuleWindow::Update()
{
	update_status ret = UPDATE_CONTINUE;

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT) ret = UPDATE_STOP;
	}

	return ret;
}

// Called before quitting
bool ModuleWindow::CleanUp()
{
	if (window) SDL_DestroyWindow(window);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	return true;
}
