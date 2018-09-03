#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"

struct SDL_Window;
struct SDL_Surface;

class ModuleWindow : public Module
{
public:

	ModuleWindow(const char* name, bool start_enabled = true);
	~ModuleWindow();

	bool Init(JSONNode* config_module = nullptr) override;
	bool CleanUp() override;

	void RecieveEvent(const Event* e) override;

	SDL_Window* GetWindow() const;

public:

	SDL_Window* window;
	SDL_Surface* screen_surface;

private:

	uint flags;

};

#endif // __ModuleWindow_H__