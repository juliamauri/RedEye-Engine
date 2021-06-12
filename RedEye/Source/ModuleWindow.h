#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"
#include <EASTL/string.h>

struct SDL_Window;

class ModuleWindow : public Module
{
public:

	ModuleWindow() : Module("Window") {}
	~ModuleWindow() {}

	bool Init() override;
	void DrawEditor() override;
	void CleanUp() override;

	void RecieveEvent(const Event& e) override;

	void Load() override;
	void Save() const override;

	SDL_Window* GetWindow() const;

	int GetWidth() const;
	int GetHeight() const;
	int GetMaxWidth() const;
	int GetMaxHeight() const;
	float GetAspectRatio() const;

	float GetBrightness()const;
	bool CheckFlag(const unsigned int flag) const;

	void SetBrightness(const float brightness);
	void SetTitle(const char* new_title = nullptr);
	void SetWindowSize(unsigned int new_width, unsigned int new_height);

	void SetResizeable(const bool flag_value);
	void SetFullScreen(const bool flag_value);
	void SetBorderless(const bool flag_value);
	void SetFullDesktop(const bool flag_value);
	void SetWindowPos(int x, int y);

	void SwapResizeable();
	void SwapFullScreen();
	void SwapBorderless();
	void SwapFullDesktop();

private:

	bool SetWindowProperties();

public:

	SDL_Window* window = nullptr;
	struct SDL_Surface* screen_surface = nullptr;

private:

	unsigned int flags = 0u;
	int pos_x = -1, pos_y = -1;
	int width = 1280, height = 720;
	float brightness = 0.9f;
	eastl::string title = "RedEye";
};

#endif // __ModuleWindow_H__