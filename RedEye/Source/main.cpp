#include "Globals.h"
#include "Application.h"

#ifdef _DEBUG
#include "mmgr\mmgr.h"
#endif

#include "SDL2\include\SDL.h"
#pragma comment( lib, "SDL2/libx86/SDL2.lib" )
#pragma comment( lib, "SDL2/libx86/SDL2main.lib" )

Application* App = nullptr;

int main(int argc, char* argv[])
{
	int main_return = EXIT_FAILURE;

	App = new Application(argc, argv);

	if (App->Init())
	{
		int update_return = App->Update();

		while(update_return == 1)
			update_return = App->Update();

		if (update_return == 0)
		{
			if (App->CleanUp())
			{
				main_return = EXIT_SUCCESS;
				LOG("EXIT SUCCESS\n");
			}
			else
			{
				LOG("Application CleanUp exits with ERROR");
			}
		}
		else
		{
			LOG("Application Update exits with ERROR");
		}
	}
	else
	{
		LOG("Application Init exits with ERROR");
	}

	delete App;

#ifdef _DEBUG
	LOG("With %d memory leaks!\n", (m_getMemoryStatistics().totalAllocUnitCount));
#endif

	return main_return;
}
