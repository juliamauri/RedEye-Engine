#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "EventListener.h"

class ModuleWindow : public EventListener
{
private:

	uint flags = 0;
	int pos_x = -1;
	int pos_y = -1;
	int width = 1280;
	int height = 720;
	float brightness = 0.9f;
	eastl::string title = "RedEye";

	struct SDL_Window* window = nullptr;
	struct SDL_Surface* surface = nullptr;

public:

	ModuleWindow() = default;
	~ModuleWindow() final = default;

	// Module
	bool Init();
	void CleanUp();
	void DrawEditor();
	void RecieveEvent(const Event& e) final;
	void Load();
	void Save() const;

	// Getters
	float GetBrightness() const { return brightness; }
	SDL_Window* GetWindow() const { return window; }
	SDL_Surface* GetSurface() const { return surface; }

	int GetWidth() const;
	int GetHeight() const;
	int GetMaxWidth() const;
	int GetMaxHeight() const;
	float GetAspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }
	bool HasFlag(uint flag) const { return flags & flag; }

	// Setters
	void SetBrightness(float brightness);
	void SetTitle(const char* new_title);
	void SetWindowSize(uint new_width, uint new_height);
	void SetResizeable(bool flag_value);
	void SetFullScreen(bool flag_value);
	void SetBorderless(bool flag_value);
	void SetFullDesktop(bool flag_value);
	void SetWindowPos(int x, int y);

	// Swap Setters
	void SwapResizeable();
	void SwapFullScreen();
	void SwapBorderless();
	void SwapFullDesktop();

private:

	void CreateWindow();
	bool SetWindowProperties();
};

#endif // __ModuleWindow_H__