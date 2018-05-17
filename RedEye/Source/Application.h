#ifndef __APP_H__
#define __APP_H__

#include <list>

class Module;
class ModuleWindow;
class ModuleInput;

class Application
{
public:
	Application();
	~Application();

	bool Init();
	int Update();
	bool CleanUp();

private:

	void PrepareUpdate();
	void FinishUpdate();

public:

	ModuleWindow* window = nullptr;
	ModuleInput* input = nullptr;

private:

	std::list<Module*> modules;
};

extern Application* App;

#endif // __APP_H__