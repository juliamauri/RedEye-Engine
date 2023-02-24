#include <SDL2/SDL.h>

#include "JR_Application.h"

int main(int argc, char* argv[])
{
	APP = new JR_Application();
	if (APP->Init())
	{
		APP->Update();
		APP->CleanUp();
	}
	delete APP;
	return 0;
}