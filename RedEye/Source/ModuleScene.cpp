#include "ModuleScene.h"

#include <string>

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::CleanUp()
{
	return true;
}

void ModuleScene::RecieveEvent(const Event * e)
{
	bool t;
	//std::string fn = e->GetData()->ReadAsCharP();
	//std::string ext = fn.substr(fn.find_last_of(".") + 1);

	//LOG("File - %s - dropped", fn.c_str());
}
