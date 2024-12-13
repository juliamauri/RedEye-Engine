#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "EventListener.h"

class ModuleWindow : public EventListener
{
public:

	ModuleWindow() = default;
	~ModuleWindow() final = default;

	bool Init();
	void CleanUp();

	void DrawEditor();
	void RecieveEvent(const Event& e) final;

	void Load();
	void Save() const;

	struct SDL_Window* GetWindow() const;

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