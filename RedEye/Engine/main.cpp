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

/*
void* operator new(std::size_t sz)
{
	std::printf("1) new(size_t), size = %zu\n", sz);
	if (sz == 0)
		++sz; // avoid std::malloc(0) which may return nullptr on success

	if (void *ptr = std::malloc(sz))
		return ptr;

	throw std::bad_alloc{}; // required by [new.delete.single]/3
}

void* operator new[](std::size_t sz)
{
	std::printf("2) new[](size_t), size = %zu\n", sz);
	if (sz == 0)
		++sz; // avoid std::malloc(0) which may return nullptr on success

	if (void *ptr = std::malloc(sz))
		return ptr;

	throw std::bad_alloc{}; // required by [new.delete.single]/3
}
*/

void operator delete(void* ptr, std::size_t size) noexcept
{
	//std::printf("4) delete(void*, size_t), size = %zu\n", size);
	std::free(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept
{
	//std::printf("6) delete[](void*, size_t), size = %zu\n", size);
	std::free(ptr);
}

// Declare and define unique global variable
#include "Application.h"
Application* App = nullptr;


int main(int argc, char* argv[])
{
	bool exit_with_error = true;
	App = new("I'm Applicaciton", 0, 0, __FILE__, __LINE__) Application();

	if (App->Init(argc, argv))
	{
		App->MainLoop();
		App->CleanUp();
		exit_with_error = false;
	}

	delete App;
	return exit_with_error;
}