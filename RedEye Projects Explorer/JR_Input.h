#ifndef JR_INPUT_CLASS
#define JR_INPUT_CLASS

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_stdinc.h>

class JR_Input
{
public:
	bool Init();

	bool PreUpdate();

	void CleanUp();

public:
	static JR_Input* instance;


private:
	SDL_Event event;
};

#endif // !JR_INPUT_CLASS