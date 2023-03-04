#ifndef JR_WINDOWSMANAGER_CLASS
#define JR_WINDOWSMANAGER_CLASS

#include <EASTL/vector.h>

class JR_WindowsManager
{
public:
	bool Init();

	void Draw();

	void CleanUp();

private:
	void SetStyleColor();

	eastl::vector<class JR_Window*> windows;
};

#endif // !JR_MAINWINDOWS_CLASS