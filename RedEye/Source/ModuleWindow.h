#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"

struct SDL_Window;
struct SDL_Surface;
union SDL_Event;

class ModuleWindow : public Module
{
public:

	ModuleWindow(const char* name, bool start_enabled = true);
	~ModuleWindow();

	bool Init(JSONNode* node = nullptr) override;
	void DrawEditor() override;
	bool CleanUp() override;

	void RecieveEvent(const Event* e) override;
	void WindowEvent(const SDL_Event* e);

	SDL_Window* GetWindow() const;

	int GetWidth() const;
	int GetHeight() const;
	int GetMaxWidth() const;
	int GetMaxHeight() const;

	float	GetBrightness()const;
	bool	CheckFlag(uint flag) const;

	void SetBrightness(const float brightness);
	void SetTitle(const char* new_title = nullptr);
	void SetWindowSize(unsigned int new_width, unsigned int new_height);

	void SetResizeable(const bool flag_value);
	void SetFullScreen(const bool flag_value);
	void SetBorderless(const bool flag_value);
	void SetFullDesktop(const bool flag_value);

	void SwapResizeable();
	void SwapFullScreen();
	void SwapBorderless();
	void SwapFullDesktop();

public:

	SDL_Window* window = nullptr;
	SDL_Surface* screen_surface = nullptr;

private:

	unsigned int flags = 0u;

	int width = 800;
	int height = 600;
};

#endif // __ModuleWindow_H__