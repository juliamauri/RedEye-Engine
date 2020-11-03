#include "Application.h"
#include "OutputLog.h"

/*  HELLO EASTL! 
//How to track the memory:
https://stackoverflow.com/questions/42565582/how-to-track-memory-usage-using-eastlç
https://www.swardle.com/sweb/img/Memory%20and%20C++%20debuging%20at%20EA%20-%20Scott%20Wardle%20-%20CppCon%202015.pdf
*/

#include "SDL2\include\SDL.h"
#pragma comment( lib, "SDL2/libx86/SDL2.lib" )
#pragma comment( lib, "SDL2/libx86/SDL2main.lib" )

#include "Optick/include/optick.h"
#ifdef _DEBUG
#pragma comment( lib, "Optick/libx86/OptickCore_debug.lib" )
#pragma comment( lib, "EA/EASTL/libx86/DebugLib/EASTL.lib" )
#else
#pragma comment( lib, "Optick/libx86/OptickCore_release.lib" )
#pragma comment( lib, "EA/EASTL/libx86/ReleaseLib/EASTL.lib" )
#endif

void* operator new[](size_t size, const char* pName, int flags, unsigned     debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

int main(int argc, char* argv[])
{
	int main_return = EXIT_FAILURE;

	Application* app = new Application();

	RE_LOG_SEPARATOR("Initializing RedEye");

	if (app->Init(argc, argv))
	{
		int update_return = app->Update();

		RE_LOG_SEPARATOR("Entering Application's Main Loop");

		while (update_return == 1)
		{
			OPTICK_FRAME("MainThread RedEye");
			update_return = app->Update();
		}

		if (update_return == 0)
		{
			RE_LOG_SEPARATOR("Cleaning Up Application");

			if (app->CleanUp())
			{
				main_return = EXIT_SUCCESS;
				RE_LOG_SEPARATOR("EXIT SUCCESS");
			}
			else
			{
				RE_LOG_ERROR("Application CleanUp exits with ERROR");
			}
		}
		else
		{
			RE_LOG_ERROR("Application Update exits with ERROR");
		}
	}
	else
	{
		RE_LOG_ERROR("Application Init exits with ERROR");
	}

	DEL(app);

	return main_return;
}
