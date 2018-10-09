#include "Application.h"
#include "OutputLog.h"

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

	LOG_SEPARATOR("Initializing RedEye");

	if (App->Init())
	{
		int update_return = App->Update();

		LOG_SEPARATOR("Entering Application's Main Loop");

		while(update_return == 1)
			update_return = App->Update();

		if (update_return == 0)
		{
			LOG_SEPARATOR("Cleaning Up Application");

			if (App->CleanUp())
			{
				main_return = EXIT_SUCCESS;
				LOG_SEPARATOR("EXIT SUCCESS");
			}
			else
			{
				LOG_ERROR("Application CleanUp exits with ERROR");
			}
		}
		else
		{
			LOG_ERROR("Application Update exits with ERROR");
		}
	}
	else
	{
		LOG_ERROR("Application Init exits with ERROR");
	}

	delete App;
	App = nullptr;

#ifdef _DEBUG
	LOG("With %d memory leaks!\n", (m_getMemoryStatistics().totalAllocUnitCount));
#endif

	return main_return;
}
