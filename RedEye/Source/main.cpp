#include "Application.h"

#include "SDL2\include\SDL.h"
#pragma comment( lib, "SDL2/libx86/SDL2.lib" )
#pragma comment( lib, "SDL2/libx86/SDL2main.lib" )

#ifdef _DEBUG
#pragma comment( lib, "Optick/libx86/OptickCore_debug.lib" )
#pragma comment( lib, "EA/EASTL/libx86/DebugLib/EASTL.lib" )
#else
#pragma comment( lib, "Optick/libx86/OptickCore_release.lib" )
#pragma comment( lib, "EA/EASTL/libx86/ReleaseLib/EASTL.lib" )
#endif

/*  HELLO EASTL!
//How to track the memory:
https://stackoverflow.com/questions/42565582/how-to-track-memory-usage-using-eastlç
https://www.swardle.com/sweb/img/Memory%20and%20C++%20debuging%20at%20EA%20-%20Scott%20Wardle%20-%20CppCon%202015.pdf*/
void* operator new[](size_t size, const char* pName, int flags, unsigned     debugFlags, const char* file, int line)
{ return new uint8_t[size]; }
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{ return new uint8_t[size]; }

// Declare and define unique global variable
Application* App = nullptr;

int main(int argc, char* argv[])
{
	bool exit_with_error = true;
	App = new Application();

	if (App->Init(argc, argv))
	{
		App->MainLoop();
		App->CleanUp();
		exit_with_error = false;
	}

	delete App;
	return exit_with_error;
}