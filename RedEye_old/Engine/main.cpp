// Build instructions
#define _STATIC_CPPLIB

// Disable STL exceptions
#undef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0

// Disable depricate code warnings
#define _DISABLE_DEPRECATE_STATIC_CPPLIB
//#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <SDL2/SDL.h>

#include <cstdlib>

/*  HELLO EASTL! -> How to track the memory:
https://stackoverflow.com/questions/42565582/how-to-track-memory-usage-using-eastlç
https://www.swardle.com/sweb/img/Memory%20and%20C++%20debuging%20at%20EA%20-%20Scott%20Wardle%20-%20CppCon%202015.pdf*/

void* operator new[](size_t size, const char* pName, int flags, unsigned     debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}


void* operator new (size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

// Declare and define unique global variable
#include "Application.h"
Application* App = nullptr;

int main(int argc, char* argv[])
{
	bool exit_with_error = true;
	App = new Application();
	App->AllocateModules();

	if (App->Init(argc, argv))
	{
		App->MainLoop();
		App->CleanUp();
		exit_with_error = false;
	}

	delete App;
	return exit_with_error;
}