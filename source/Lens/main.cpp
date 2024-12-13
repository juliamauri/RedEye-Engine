#include <SDL2/SDL.h>
#include "RL_Application.h"

int main(int argc, char* argv[])
{
	APP = new JR_Application();
	if (APP->Init(argv))
	{
		APP->Update();
		APP->CleanUp();
	}
	delete APP;
	return 0;
}