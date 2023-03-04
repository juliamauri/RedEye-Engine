#include "Event.h"

#include "ModuleWindow.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_Json.h"
#include "ModuleRenderer3D.h"

#include <ImGui/imgui.h>
#include <SDL2/SDL.h>
#include <RapidJson/document.h>

bool ModuleWindow::Init()
{
	bool ret = false;
	RE_PROFILE(PROF_Init, PROF_ModuleWindow);
	RE_LOG("Initializing Module Window");
	RE_LOG_SECONDARY("Init SDL video subsystem");
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == 0)
	{
		Load();
		ret = SetWindowProperties();
	}
	else RE_LOG_ERROR("SDL_VIDEO could not initialize! SDL_Error: %s", SDL_GetError());

	return ret;
}

void ModuleWindow::DrawEditor()
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
		ImGui::Text("Width: %u", width);
		ImGui::Text("Height: %u", height);
	}

	ImGui::Text("Position X: %u", pos_x);
	ImGui::Text("Position Y: %u", pos_y);

	ImGui::Separator();
	ImGui::Text("Window Flags");

	bool flag = CheckFlag(SDL_WINDOW_RESIZABLE);
	if (ImGui::Checkbox("Resizeable", &flag)) SetResizeable(flag);

	ImGui::SameLine();
	flag = CheckFlag(SDL_WINDOW_FULLSCREEN);
	if (ImGui::Checkbox("Fullscreen", &flag)) SetFullScreen(flag);

	flag = CheckFlag(SDL_WINDOW_BORDERLESS);
	if (ImGui::Checkbox("View Border", &flag)) SetBorderless(flag);

	ImGui::SameLine();
	flag = CheckFlag(SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (ImGui::Checkbox("Fullscreen Desktop", &flag)) SetFullDesktop(flag);

	ImGui::Separator();
	if (ImGui::SliderFloat("Brightness", &brightness, 0.f, 1.f, "%.2f")) SetBrightness(brightness);
}

void ModuleWindow::CleanUp()
{
	RE_PROFILE(PROF_CleanUp, PROF_ModuleWindow);
	if (window) SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void ModuleWindow::Load()
{
	RE_PROFILE(PROF_Load, PROF_ModuleWindow);
	RE_LOG_SECONDARY("Loading Window propieties from config:");
	RE_Json* node = RE_FS->ConfigNode("Window");

	/*/Use OpenGL 2.1 ?? TODO: check prefered GL Context version setting
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);*/

	//OpenGL context 
	flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if (node->PullBool("fullscreen", false)) flags |= SDL_WINDOW_FULLSCREEN;
	if (node->PullBool("resizable", true)) flags |= SDL_WINDOW_RESIZABLE;
	if (node->PullBool("borderless", false)) flags |= SDL_WINDOW_BORDERLESS;
	if (node->PullBool("fullscreen_desktop", false)) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	title = node->PullString("title", title.c_str());
	pos_x = node->PullInt("pos_x", SDL_WINDOWPOS_CENTERED);
	pos_y = node->PullInt("pos_y", SDL_WINDOWPOS_CENTERED);
	width = node->PullInt("width", width);
	height = node->PullInt("height", height);

	DEL(node);
}

void ModuleWindow::Save() const
{
	RE_PROFILE(PROF_Save, PROF_ModuleWindow);
	RE_Json* node = RE_FS->ConfigNode("Window");
	if (flags == 0u)
	{
		node->PushBool("Fullscreen", false);
		node->PushBool("Resizable", true);
		node->PushBool("Borderless", false);
		node->PushBool("Fullscreen Desktop", false);
	}
	else
	{
		node->PushBool("Fullscreen", flags & SDL_WINDOW_FULLSCREEN);
		node->PushBool("Resizable", flags & SDL_WINDOW_RESIZABLE);
		node->PushBool("Borderless", flags & SDL_WINDOW_BORDERLESS);
		node->PushBool("Fullscreen Desktop", flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
	}

	node->PushString("title", title.c_str());
	node->PushInt("pos_x", pos_x);
	node->PushInt("pos_y", pos_y);
	node->PushInt("width", width);
	node->PushInt("height", height);

	DEL(node);
}

void ModuleWindow::RecieveEvent(const Event& e)
{
	switch (e.type)
	{
	case RE_EventType::WINDOW_MOVED:
	{
		pos_x = e.data1.AsInt();
		pos_y = e.data2.AsInt();
		break;
	}
	case RE_EventType::WINDOW_SIZE_CHANGED:
	{
		width = e.data1.AsInt();
		height = e.data2.AsInt();
		break;
	}
	}
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

float ModuleWindow::GetAspectRatio() const
{
	return static_cast<float>(width) / static_cast<float>(height);
}

float ModuleWindow::GetBrightness()const
{
	return brightness;
}

bool ModuleWindow::CheckFlag(const unsigned int flag) const
{
	return flags & flag;
}


void ModuleWindow::SetBrightness(const float b)
{
	brightness = b;
	SDL_SetWindowBrightness(window, b);
}

void ModuleWindow::SetTitle(const char* new_title)
{
	if (new_title != nullptr)
	{
		title = new_title;
		SDL_SetWindowTitle(window, new_title);
	}
}

void ModuleWindow::SetWindowSize(unsigned int new_width, unsigned int new_height)
{
	SDL_SetWindowSize(window, width = new_width, height = new_height);
}

void ModuleWindow::SetResizeable(const bool flag_value)
{
	if (static_cast<bool>(flags & SDL_WINDOW_RESIZABLE) != flag_value)
	{
		if (flag_value)
			flags |= SDL_WINDOW_RESIZABLE;
		else
			flags -= SDL_WINDOW_RESIZABLE;

		SDL_SetWindowResizable(window, SDL_bool(flag_value));
	}
}

void ModuleWindow::SetFullScreen(const bool flag_value)
{
	if (static_cast<bool>(flags & SDL_WINDOW_FULLSCREEN) != flag_value)
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

void ModuleWindow::SetBorderless(const bool flag_value)
{
	if (static_cast<bool>(flags & SDL_WINDOW_BORDERLESS) != flag_value)
	{
		if (flag_value)
			flags |= SDL_WINDOW_BORDERLESS;
		else
			flags -= SDL_WINDOW_BORDERLESS;

		SDL_SetWindowBordered(window, SDL_bool(flag_value));
	}
}

void ModuleWindow::SetFullDesktop(const bool flag_value)
{
	if (static_cast<bool>(flags & SDL_WINDOW_FULLSCREEN) != flag_value)
	{
		if (flag_value)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		else
			flags -= SDL_WINDOW_FULLSCREEN_DESKTOP;

		SDL_SetWindowFullscreen(window, flag_value ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_FALSE);
	}
}

void ModuleWindow::SetWindowPos(const int x, const int y)
{
	SDL_SetWindowPosition(window, pos_x = x, pos_y = y);
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

bool ModuleWindow::SetWindowProperties()
{
	RE_PROFILE(PROF_SetWindowProperties, PROF_ModuleWindow);
	bool ret = false;
	if (window == nullptr)
	{
		RE_PROFILE(PROF_CreateWindow, PROF_ModuleWindow);
		RE_LOG_SECONDARY("Creating new window: %s | Width: %i | Height: %i", title.c_str(), width, height);
		window = SDL_CreateWindow(title.c_str(), pos_x, pos_y, width, height, flags);
	}

	if (window != nullptr)
	{
		eastl::string w_title = title;
#ifdef _WIN64
		w_title += " (Engine x64)";
#endif // _DEBUG

		SDL_SetWindowTitle(window, w_title.c_str());
		SDL_SetWindowResizable(window, static_cast<SDL_bool>(flags & SDL_WINDOW_RESIZABLE));
		SDL_SetWindowBordered(window, static_cast<SDL_bool>(!(flags & SDL_WINDOW_BORDERLESS)));
		SDL_SetWindowPosition(window, pos_x, pos_y);
		SDL_SetWindowSize(window, width, height);

		bool fullscreen = (flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
		if (SDL_SetWindowFullscreen(window, static_cast<SDL_bool>(fullscreen)) < 0)
			RE_LOG_WARNING("SDL Window was not able to set fullscreen to %b", fullscreen);

		if (SDL_SetWindowBrightness(window, brightness) < 0)
			RE_LOG_WARNING("SDL Window was not able to set brightness to %f", brightness);

		screen_surface = SDL_GetWindowSurface(window);
		ret = (screen_surface != nullptr);

	}
	else RE_LOG_ERROR("Window could not be created! SDL_Error: %s\n", SDL_GetError());

	return ret;
}
