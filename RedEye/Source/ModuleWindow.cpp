#include "ModuleWindow.h"

//#include "Application.h"
#include "Event.h"
#include "SDL2\include\SDL.h"

#include "FileSystem.h"
#include "RapidJson\include\document.h"


ModuleWindow::ModuleWindow(const char* name, bool start_enabled) : Module(name, start_enabled),
window(nullptr), screen_surface(nullptr), flags(0)
{}

// Destructor
ModuleWindow::~ModuleWindow()
{}

// Called before render is available
bool ModuleWindow::Init(JSONNode* config_module)
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

		const char* title = config_module->PullString("title", "no se ha cargado");
		uint screen_width = config_module->PullUInt("screen_width", 800);
		uint screen_height = config_module->PullUInt("screen_height", 800);
		bool fullscreen = config_module->PullBool("fullscreen", false);
		bool resizable = config_module->PullBool("resizable", false);
		bool borderless = config_module->PullBool("borderless", false);
		bool fullscreen_desktop = config_module->PullBool("fullscreen_desktop", false);

		if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
		if (resizable) flags |= SDL_WINDOW_RESIZABLE;
		if (borderless) flags |= SDL_WINDOW_BORDERLESS;
		if (fullscreen_desktop) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		//OpenGL context 
		flags |= SDL_WINDOW_OPENGL;

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

// Called before quitting
bool ModuleWindow::CleanUp()
{
	if (window) SDL_DestroyWindow(window);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	return true;
}

void ModuleWindow::RecieveEvent(const Event* e)
{
}

SDL_Window * ModuleWindow::GetWindow() const
{
	return window;
}
