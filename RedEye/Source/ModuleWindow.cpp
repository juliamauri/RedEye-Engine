#include "ModuleWindow.h"

#include "FileSystem.h"
#include "Event.h"
#include "ImGui\imgui.h"
#include "SDL2\include\SDL.h"
#include "RapidJson\include\document.h"

ModuleWindow::ModuleWindow(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleWindow::~ModuleWindow()
{}

bool ModuleWindow::Init(JSONNode* node)
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

		//const char* title = node->PullString("title", "no se ha cargado");
		width = node->PullUInt("screen_width", width);
		height = node->PullUInt("screen_height", height);

		if (node->PullBool("fullscreen", true))
			flags |= SDL_WINDOW_FULLSCREEN;
		if (node->PullBool("resizable", true))
			flags |= SDL_WINDOW_RESIZABLE;
		if (node->PullBool("borderless", false))
			flags |= SDL_WINDOW_BORDERLESS;
		if (node->PullBool("fullscreen_desktop", false))
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		//OpenGL context 
		flags |= SDL_WINDOW_OPENGL;

		//Create window
		window = SDL_CreateWindow(
			node->PullString("title", "no se ha cargado"), // Title
			SDL_WINDOWPOS_CENTERED, // x position
			SDL_WINDOWPOS_CENTERED, // y position
			width, // width
			height, // height
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

void ModuleWindow::DrawEditor()
{
	if (ImGui::CollapsingHeader("Window"))
	{

		int max_w = GetMaxWidth(), max_h = GetMaxHeight();
		ImGui::Text("Screen Size: %u x %u", max_w, max_h);

		if (CheckFlag(SDL_WINDOW_RESIZABLE))
		{
			if (ImGui::SliderInt("Width", &width, 0, max_w, "%.0f"))
				SetWindowSize(width, height);
			if (ImGui::SliderInt("Height", &height, 0, max_h, "%.0f"))
				SetWindowSize(width, height);
		}
		else
		{
			ImGui::Text("Width: %u\t", width);
			ImGui::Text("Height: %u", height);
		}

		ImGui::Separator();

		ImGui::Text("Window Flags");

		bool flag = CheckFlag(SDL_WINDOW_RESIZABLE);
		if (ImGui::Checkbox("Resizeable", &flag))
			SetResizeable(flag);
		ImGui::SameLine();
		flag = CheckFlag(SDL_WINDOW_FULLSCREEN);
		if (ImGui::Checkbox("Fullscreen", &flag))
			SetFullScreen(flag);

		flag = CheckFlag(SDL_WINDOW_BORDERLESS);
		if (ImGui::Checkbox("View Border", &flag))
			SetBorderless(flag);
		ImGui::SameLine();
		flag = CheckFlag(SDL_WINDOW_FULLSCREEN_DESKTOP);
		if (ImGui::Checkbox("Fullscreen Desktop", &flag))
			SetFullDesktop(flag);
		
		ImGui::Separator();

		float brightness = GetBrightness();
		if (ImGui::SliderFloat("Brightness", &brightness, 0.f, 1.f, "%.2f"))
			SetBrightness(brightness);
	}
}

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

int ModuleWindow::GetWidth() const
{
	return (flags & SDL_WINDOW_FULLSCREEN) ? GetMaxWidth() : width;
}

int ModuleWindow::GetHeight() const
{
	return (flags & SDL_WINDOW_FULLSCREEN) ? GetMaxHeight() : height;
}

int ModuleWindow::GetMaxWidth() const
{
	int ret = -1;

	SDL_DisplayMode mode;
	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
	{
		if (SDL_GetCurrentDisplayMode(i, &mode) == 0)
			ret = mode.w;
	}

	return ret;
}

int ModuleWindow::GetMaxHeight() const
{
	int ret = -1;

	SDL_DisplayMode mode;
	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
	{
		if (SDL_GetCurrentDisplayMode(i, &mode) == 0)
			ret = mode.h;
	}

	return ret;
}

float ModuleWindow::GetBrightness()const
{
	return SDL_GetWindowBrightness(window);
}

bool ModuleWindow::CheckFlag(uint flag) const
{
	return flags & flag;
}


void ModuleWindow::SetBrightness(const float brightness)
{
	SDL_SetWindowBrightness(window, brightness);
}

void ModuleWindow::SetTitle(const char* new_title)
{
	if (new_title != nullptr)
	{
		SDL_SetWindowTitle(window, new_title);
	}
}

void ModuleWindow::SetWindowSize(unsigned int new_width, unsigned int new_height)
{
	width = new_width;
	height = new_height;

	if (flags & SDL_WINDOW_RESIZABLE)
		SDL_SetWindowSize(window, new_width, new_height);
}

void ModuleWindow::SetResizeable(const bool flag_value)
{
	if ((flags & SDL_WINDOW_RESIZABLE) != flag_value)
	{
		if (flag_value)
			flags |= SDL_WINDOW_RESIZABLE;
		else
			flags -= SDL_WINDOW_RESIZABLE;

		SDL_SetWindowResizable(window, SDL_bool(flag_value));
	}
}

void ModuleWindow::SetFullScreen(bool flag_value)
{
	if ((flags & SDL_WINDOW_FULLSCREEN) != flag_value)
	{
		if (flag_value)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
			SDL_SetWindowSize(window, GetMaxWidth(), GetMaxHeight());
		}
		else
		{
			flags -= SDL_WINDOW_FULLSCREEN;
			SDL_SetWindowSize(window, width, height);
		}

		
		SDL_SetWindowFullscreen(window, SDL_bool(flag_value));
	}
}

void ModuleWindow::SetBorderless(bool flag_value)
{
	if ((flags & SDL_WINDOW_BORDERLESS) != flag_value)
	{
		if (flag_value)
			flags |= SDL_WINDOW_BORDERLESS;
		else
			flags -= SDL_WINDOW_BORDERLESS;

		SDL_SetWindowBordered(window, SDL_bool(flag_value));
	}
}

void ModuleWindow::SetFullDesktop(bool flag_value)
{
	if ((flags & SDL_WINDOW_FULLSCREEN) != flag_value)
	{
		if (flag_value)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		else
			flags -= SDL_WINDOW_FULLSCREEN_DESKTOP;

		SDL_SetWindowFullscreen(window, flag_value ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_FALSE);
	}
}


void ModuleWindow::SwapResizeable()
{
	SDL_SetWindowResizable(window, SDL_bool(CheckFlag(SDL_WINDOW_RESIZABLE) ? 0 : 1));
}
void ModuleWindow::SwapFullScreen()
{
	SetFullScreen(!(CheckFlag(SDL_WINDOW_FULLSCREEN)));
}

void ModuleWindow::SwapBorderless()
{
	SetBorderless(!(CheckFlag(SDL_WINDOW_BORDERLESS)));
}

void ModuleWindow::SwapFullDesktop()
{
	SetFullDesktop(!(CheckFlag(SDL_WINDOW_FULLSCREEN_DESKTOP)));
}
