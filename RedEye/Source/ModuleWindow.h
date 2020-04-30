#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"
#include <EASTL/string.h>

struct SDL_Window;
class SDL_Surface;
union SDL_Event;

class ModuleWindow : public Module
{
public:

	ModuleWindow(const char* name, bool start_enabled = true);
	~ModuleWindow();

	bool Init(JSONNode* node) override;
	void DrawEditor() override;
	bool CleanUp() override;
	bool Load(JSONNode* node) override;
	bool Save(JSONNode* node) const override;
	void RecieveEvent(const Event& e) override;

	SDL_Window* GetWindow() const;

	int GetWidth() const;
	int GetHeight() const;
	int GetMaxWidth() const;
	int GetMaxHeight() const;
	float GetAspectRatio() const;

	float	GetBrightness()const;
	bool	CheckFlag(uint flag) const;

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

	void SetWindowProperties();

public:

	SDL_Window* window = nullptr;
	SDL_Surface* screen_surface = nullptr;

private:

	eastl::string title = "RedEye";
	int pos_x;
	int pos_y;
	int width = 1280;
	int height = 720;
	unsigned int flags = 0u;
	float brightness = 0.9f;
};

#endif // __ModuleWindow_H__