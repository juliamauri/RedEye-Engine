#include "ModuleWindow.h"

#include "Application.h"
#include "ModuleInput.h"
#include "FileSystem.h"
#include "ModuleRenderer3D.h"
#include "Event.h"
#include "OutputLog.h"
#include "ImGui\imgui.h"
#include "SDL2\include\SDL.h"
#include "RapidJson\include\document.h"

ModuleWindow::ModuleWindow(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleWindow::~ModuleWindow()
{}

bool ModuleWindow::Init(JSONNode* node)
{
	LOG_SECONDARY("Init SDL video subsystem");
	bool ret = true;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		LOG_ERROR("SDL_VIDEO could not initialize! SDL_Error: %s", SDL_GetError());
		ret = false;
	}
	else
	{
		//Use OpenGL 2.1 ?? TODO check with teacher
		/*SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);*/

		pos_x = pos_y = SDL_WINDOWPOS_CENTERED;

		Load(node);
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
			ImGui::Text("Width: %u", width);
			ImGui::Text("Height: %u", height);
		}

		ImGui::Text("Position X: %u", pos_x);
		ImGui::Text("Position Y: %u", pos_y);

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

bool ModuleWindow::Load(JSONNode * node)
{
	bool ret = true;

	LOG_SECONDARY("Loading Window propieties from config:");

	//OpenGL context 
	flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if (node->PullBool("fullscreen", false))
		flags |= SDL_WINDOW_FULLSCREEN;
	if (node->PullBool("resizable", true))
		flags |= SDL_WINDOW_RESIZABLE;
	if (node->PullBool("borderless", false))
		flags |= SDL_WINDOW_BORDERLESS;
	if (node->PullBool("fullscreen_desktop", false))
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	title = node->PullString("title", title.c_str());
	pos_x = node->PullInt("pos_x", pos_x);
	pos_y = node->PullInt("pos_y", pos_y);
	width = node->PullInt("width", width);
	height = node->PullInt("height", height);

	SetWindowProperties();

	if (!window)
	{
		LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		brightness = SDL_GetWindowBrightness(window);
		screen_surface = SDL_GetWindowSurface(window);
	}

	return ret;
}

bool ModuleWindow::Save(JSONNode * node) const
{
	bool ret = true;

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

	return ret;
}

void ModuleWindow::RecieveEvent(const Event& e)
{
	
}

void ModuleWindow::WindowEvent(const SDL_Event * e)
{
	switch (e->window.event)
	{
	case SDL_WINDOWEVENT_SHOWN:/**< Window has been shown */
		break;
	case SDL_WINDOWEVENT_HIDDEN:/**< Window has been hidden */
		break;
	case SDL_WINDOWEVENT_EXPOSED:/**< Window has been exposed and should be redrawn */
		break;
	case SDL_WINDOWEVENT_MOVED:/**< Window has been moved to data1, data2 */
		pos_x = e->window.data1;
		pos_y = e->window.data2;
		break;
	case SDL_WINDOWEVENT_RESIZED:/**< Window has been resized to data1xdata2 */
		SetWindowSize(e->window.data1, e->window.data2);
		break;
	case SDL_WINDOWEVENT_SIZE_CHANGED:/**< The window size has changed, either as a result of an API call or through the system or user changing the window size. */
		width = e->window.data1;
		height = e->window.data2;
		break;
	case SDL_WINDOWEVENT_MINIMIZED:/**< Window has been minimized */
		break;
	case SDL_WINDOWEVENT_MAXIMIZED:/**< Window has been maximized */
		break;
	case SDL_WINDOWEVENT_RESTORED:/**< Window has been restored to normal size and position */
		break;
	case SDL_WINDOWEVENT_ENTER:/**< Window has gained mouse focus */
		break;
	case SDL_WINDOWEVENT_LEAVE:/**< Window has lost mouse focus */
		break;
	case SDL_WINDOWEVENT_FOCUS_GAINED:/**< Window has gained keyboard focus */
		break;
	case SDL_WINDOWEVENT_FOCUS_LOST:/**< Window has lost keyboard focus */
		break;
	case SDL_WINDOWEVENT_CLOSE:/**< The window manager requests that the window be closed */
		App->input->AddEvent(Event(REQUEST_QUIT, e->window.timestamp, App));
		break;
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
	return (float)width / (float)height;
}

float ModuleWindow::GetBrightness()const
{
	return brightness;
}

bool ModuleWindow::CheckFlag(uint flag) const
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
	width = new_width;
	height = new_height;

	SDL_SetWindowSize(window, new_width, new_height);
	App->renderer3d->ResetAspectRatio();
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
		App->renderer3d->ResetAspectRatio();
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
		App->renderer3d->ResetAspectRatio();
	}
}

void ModuleWindow::SetWindowPos(int x, int y)
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

void ModuleWindow::SetWindowProperties()
{
	if (window == nullptr)
	{
		LOG_SECONDARY("Creating new window: %s | Width: %i | Height: %i", title.c_str(), width, height);
		window = SDL_CreateWindow(title.c_str(), pos_x, pos_y, width, height, flags);
	}
	else
	{
		SDL_SetWindowTitle(window, title.c_str());
		SDL_SetWindowResizable(window, SDL_bool(flags & SDL_WINDOW_RESIZABLE));
		SDL_SetWindowFullscreen(window, SDL_bool(flags & SDL_WINDOW_FULLSCREEN));
		SDL_SetWindowBordered(window, SDL_bool(flags & SDL_WINDOW_BORDERLESS));
		SDL_SetWindowFullscreen(window, SDL_bool(flags & SDL_WINDOW_FULLSCREEN_DESKTOP));
		SDL_SetWindowPosition(window, pos_x, pos_y);
		SDL_SetWindowSize(window, width,height);
		SDL_SetWindowBrightness(window, brightness);
	}
}
