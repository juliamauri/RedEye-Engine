#ifndef JR_WINDOWRENDERER_CLASS
#define JR_WINDOWRENDERER_CLASS

#include <SDL2/SDL_render.h>

class JR_WindowAndRenderer
{
public:
	bool Init();

	void PostUpdate();

	void CleanUp();

	SDL_Window* GetMainWindow()const;
	SDL_GLContext GetContext()const { return context; }

private:
	void EventListener(union SDL_Event* event);

private:
	uint32_t main_window = 0;
	SDL_GLContext context = nullptr;
};

#endif // !JR_WINDOWRENDERER_CLASS