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
bool ModuleWindow::Init()
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

		uint screen_width = 1280;
		uint screen_height = 1024;
		bool fullscreen = false;
		bool resizable = false;
		bool borderless = false;
		bool fullscreen_desktop = false;

		
		if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
		if (resizable) flags |= SDL_WINDOW_RESIZABLE;
		if (borderless) flags |= SDL_WINDOW_BORDERLESS;
		if (fullscreen_desktop) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		//Create window
		window = SDL_CreateWindow(
			"Hola Juli!!", // Title
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

	SDL_Quit();

	return true;
}
