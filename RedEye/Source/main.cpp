#include "Application.h"
#include "OutputLog.h"

/*  HELLO EASTL! 
  needed to disable mmgr and add the new redefinition:
void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

//How to track the memory:
https://stackoverflow.com/questions/42565582/how-to-track-memory-usage-using-eastlç
https://www.swardle.com/sweb/img/Memory%20and%20C++%20debuging%20at%20EA%20-%20Scott%20Wardle%20-%20CppCon%202015.pdf

//Example
	#include <EASTL/array.h>

	eastl::array<int, 30> test;

	test.size();

	test.begin();
*/

#ifdef _DEBUG
//#include "mmgr\mmgr.h"
#endif

#include "SDL2\include\SDL.h"
#pragma comment( lib, "SDL2/libx86/SDL2.lib" )
#pragma comment( lib, "SDL2/libx86/SDL2main.lib" )

#include "Optick/include/optick.h"
#ifdef _DEBUG
#pragma comment( lib, "Optick/libx86/OptickCore_debug.lib" )
#pragma comment( lib, "EASTL/libx86/Debug/EASTL.lib" )
#else
#pragma comment( lib, "Optick/libx86/OptickCore_release.lib" )
#pragma comment( lib, "EASTL/libx86/Release/EASTL.lib" )
#endif

Application* App = nullptr;

int main(int argc, char* argv[])
{


	int main_return = EXIT_FAILURE;

	App = new Application();

	LOG_SEPARATOR("Initializing RedEye");

	if (App->Init(argc, argv))
	{
		int update_return = App->Update();

		LOG_SEPARATOR("Entering Application's Main Loop");

		while (update_return == 1)
		{
			OPTICK_FRAME("MainThread RedEye");
			update_return = App->Update();
		}

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
	//LOG("With %d memory leaks!\n", (m_getMemoryStatistics().totalAllocUnitCount));
#endif

	return main_return;
}
