#ifndef JR_WINDOWRENDERER_CLASS
#define JR_WINDOWRENDERER_CLASS

#include <SDL2/SDL_render.h>

class JR_WindowAndRenderer
{
public:
	bool Init();

	void PostUpdate();

	void CleanUp();

	SDL_Window* GetWindow()const { return window; }
	SDL_GLContext GetContext()const { return context; }

private:
	SDL_Window* window = nullptr;
	SDL_GLContext context = nullptr;
};

#endif // !JR_WINDOWRENDERER_CLASS