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
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::ModuleWindow)
	RE_LOG("Initializing Module Window");

	RE_LOG_SECONDARY("Init SDL video subsystem");
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		RE_LOG_ERROR("SDL_VIDEO could not initialize! SDL_Error: %s", SDL_GetError());
		return false;
	}

	RE_LOG_SECONDARY("Setting Window properties.");
	Load();
	if (!SetWindowProperties())
	{
		RE_LOG_ERROR("Error setting window properties.");
		return false;
	}

	return true;
}

void ModuleWindow::DrawEditor()
{
	int max_w = GetMaxWidth(), max_h = GetMaxHeight();
	ImGui::Text("Screen Size: %u x %u", max_w, max_h);

	if (HasFlag(SDL_WINDOW_RESIZABLE))
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

	bool flag = HasFlag(SDL_WINDOW_RESIZABLE);
	if (ImGui::Checkbox("Resizeable", &flag)) SetResizeable(flag);

	flag = HasFlag(SDL_WINDOW_FULLSCREEN);
	if (ImGui::Checkbox("Fullscreen", &flag)) SetFullScreen(flag);

	flag = HasFlag(SDL_WINDOW_BORDERLESS);
	if (ImGui::Checkbox("View Border", &flag)) SetBorderless(flag);

	flag = HasFlag(SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (ImGui::Checkbox("Fullscreen Desktop", &flag)) SetFullDesktop(flag);

	ImGui::Separator();
	if (ImGui::SliderFloat("Brightness", &brightness, 0.f, 1.f, "%.2f")) SetBrightness(brightness);
}

void ModuleWindow::CleanUp()
{
	RE_PROFILE(RE_ProfiledFunc::CleanUp, RE_ProfiledClass::ModuleWindow)
	if (window) SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void ModuleWindow::Load()
{
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::ModuleWindow)
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

	DEL(node)
}

void ModuleWindow::Save() const
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::ModuleWindow)
	RE_Json* node = RE_FS->ConfigNode("Window");
	if (flags == 0)
	{
		node->Push("Fullscreen", false);
		node->Push("Resizable", true);
		node->Push("Borderless", false);
		node->Push("Fullscreen Desktop", false);
	}
	else
	{
		node->Push("Fullscreen", static_cast<bool>(flags & SDL_WINDOW_FULLSCREEN));
		node->Push("Resizable", static_cast<bool>(flags & SDL_WINDOW_RESIZABLE));
		node->Push("Borderless", static_cast<bool>(flags & SDL_WINDOW_BORDERLESS));
		node->Push("Fullscreen Desktop", static_cast<bool>(flags & SDL_WINDOW_FULLSCREEN_DESKTOP));
	}

	node->Push("title", title.c_str());
	node->Push("pos_x", pos_x);
	node->Push("pos_y", pos_y);
	node->Push("width", width);
	node->Push("height", height);

	DEL(node)
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
	default: break;
	}
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
	SDL_DisplayMode mode;
	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
		if (SDL_GetCurrentDisplayMode(i, &mode) == 0)
			return mode.w;

	return -1;
}

int ModuleWindow::GetMaxHeight() const
{
	SDL_DisplayMode mode;
	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
		if (SDL_GetCurrentDisplayMode(i, &mode) == 0)
			return mode.h;

	return -1;
}

void ModuleWindow::SetBrightness(float b)
{
	SDL_SetWindowBrightness(window, brightness = b);
}

void ModuleWindow::SetTitle(const char* new_title)
{
	if (new_title == nullptr) return;
	SDL_SetWindowTitle(window, (title = new_title).c_str());
}

void ModuleWindow::SetWindowSize(uint new_width, uint new_height)
{
	SDL_SetWindowSize(window, width = new_width, height = new_height);
}

void ModuleWindow::SetResizeable(bool flag_value)
{
	if (static_cast<bool>(flags & SDL_WINDOW_RESIZABLE) == flag_value)
		return;

	if (flag_value) flags |= SDL_WINDOW_RESIZABLE;
	else flags -= SDL_WINDOW_RESIZABLE;
	SDL_SetWindowResizable(window, SDL_bool(flag_value));
}

void ModuleWindow::SetFullScreen(bool flag_value)
{
	if (static_cast<bool>(flags & SDL_WINDOW_FULLSCREEN) == flag_value)
		return;

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

void ModuleWindow::SetBorderless(bool flag_value)
{
	if (static_cast<bool>(flags & SDL_WINDOW_BORDERLESS) == flag_value)
		return;

	if (flag_value) flags |= SDL_WINDOW_BORDERLESS;
	else flags -= SDL_WINDOW_BORDERLESS;
	SDL_SetWindowBordered(window, SDL_bool(flag_value));
}

void ModuleWindow::SetFullDesktop(bool flag_value)
{
	if (static_cast<bool>(flags & SDL_WINDOW_FULLSCREEN) == flag_value)
		return;

	if (flag_value) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	else flags -= SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDL_SetWindowFullscreen(window, flag_value ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_FALSE);
}

void ModuleWindow::SetWindowPos(int x, int y)
{
	SDL_SetWindowPosition(window, pos_x = x, pos_y = y);
}

void ModuleWindow::SwapResizeable()
{
	SDL_SetWindowResizable(window, SDL_bool(HasFlag(SDL_WINDOW_RESIZABLE) ? 0 : 1));
}
void ModuleWindow::SwapFullScreen()
{
	SetFullScreen(!(HasFlag(SDL_WINDOW_FULLSCREEN)));
}

void ModuleWindow::SwapBorderless()
{
	SetBorderless(!(HasFlag(SDL_WINDOW_BORDERLESS)));
}

void ModuleWindow::SwapFullDesktop()
{
	SetFullDesktop(!(HasFlag(SDL_WINDOW_FULLSCREEN_DESKTOP)));
}

void ModuleWindow::CreateWindow()
{
	RE_PROFILE(RE_ProfiledFunc::CreateWindow, RE_ProfiledClass::ModuleWindow)
	RE_LOG_SECONDARY("Creating new window: %s | Width: %i | Height: %i", title.c_str(), width, height);
	window = SDL_CreateWindow(title.c_str(), pos_x, pos_y, width, height, flags);
}

bool ModuleWindow::SetWindowProperties()
{
	RE_PROFILE(RE_ProfiledFunc::SetWindowProperties, RE_ProfiledClass::ModuleWindow)

	if (!window) CreateWindow();
	if (!window)
	{
		RE_LOG_ERROR("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

#ifdef _WIN64
	SDL_SetWindowTitle(window, (title + " (Engine x64)").c_str());
#else
	SDL_SetWindowTitle(window, title.c_str());
#endif
	SDL_SetWindowResizable(window, static_cast<SDL_bool>(flags & SDL_WINDOW_RESIZABLE));
	SDL_SetWindowBordered(window, static_cast<SDL_bool>(!(flags & SDL_WINDOW_BORDERLESS)));
	SDL_SetWindowPosition(window, pos_x, pos_y);
	SDL_SetWindowSize(window, width, height);

	bool fullscreen = (flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (SDL_SetWindowFullscreen(window, static_cast<SDL_bool>(fullscreen)) < 0)
		RE_LOG_WARNING("SDL Window was not able to set fullscreen to %b", fullscreen);

	if (SDL_SetWindowBrightness(window, brightness) < 0)
		RE_LOG_WARNING("SDL Window was not able to set brightness to %f", brightness);

	surface = SDL_GetWindowSurface(window);
	if (!surface)
	{
		RE_LOG_ERROR("Window surface could not be retrieved! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	return true;
}
