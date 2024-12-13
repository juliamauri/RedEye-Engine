#include <SDL2/SDL.h>

/*  HELLO EASTL! -> How to track the memory:
https://stackoverflow.com/questions/42565582/how-to-track-memory-usage-using-eastlç
https://www.swardle.com/sweb/img/Memory%20and%20C++%20debuging%20at%20EA%20-%20Scott%20Wardle%20-%20CppCon%202015.pdf*/
void* operator new[](size_t size, const char* pName, int flags, unsigned     debugFlags, const char* file, int line)
{ return new uint8_t[size]; }
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{ return new uint8_t[size]; }

#include "JR_Application.h"

int main(int argc, char* argv[])
{
	APP = new JR_Application();
	if (APP->Init(argv))
	{
		APP->Update();
		APP->CleanUp();
	}
	delete APP;
	return 0;
}