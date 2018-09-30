#include "ModuleScene.h"

#include "Application.h"
#include "FileSystem.h"
#include "OutputLog.h"
#include <string>

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::CleanUp()
{
	return true;
}

void ModuleScene::FileDrop(const char * file)
{
	RE_FileIO* holder = App->fs->QuickBufferFromPDPath(file);
	if(holder) LOG(holder->GetBuffer());

	DEL(holder);
}

void ModuleScene::RecieveEvent(const Event * e)
{
}
