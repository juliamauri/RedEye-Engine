#include <stdlib.h>

#include <bitset>

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

	//8 bools on 1 byte
	std::bitset<8> test;

	test.set(0, true);
	test.set(1, false);
	test.set(2, true);
	test.set(3, false);
	test.set(4, true);
	test.set(5, false);
	test.set(6, true);
	test.set(7, false);

	if (test[0] == test[1])
	{
	}
	else if (test[0] == test[2])
	{
		test.test(1);
	}

	SDL_Quit();
	return 0;
}


