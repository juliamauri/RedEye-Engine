#include <stdio.h>
#include <stdlib.h>

// Old school memory leak detector and other random awesomeness
#ifdef _DEBUG
//#define TEST_MEMORY_MANAGER
#include "mmgr\mmgr.h"
#endif

#include "Globals.h"

#include "SDL2\include\SDL.h"
#pragma comment( lib, "SDL2/libx86/SDL2.lib" )
#pragma comment( lib, "SDL2/libx86/SDL2main.lib" )

int main(int argc, char* argv[])
{
	//SDL initialization without subsystems
	if (SDL_Init(0) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	SDL_Quit();


#ifdef _DEBUG
	LOG("With %d memory leaks!\n", (m_getMemoryStatistics().totalAllocUnitCount));
#endif

	return 0;
}
