#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
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

	std::string full_path(file);
	std::string ext = full_path.substr(full_path.find_last_of(".") + 1);

	if (ext.compare("fbx") == 0)
	{
		App->renderer3d->LoadNewModel(holder->GetBuffer(), file,holder->GetSize());
	}

	if(holder) LOG(holder->GetBuffer());

	DEL(holder);
}

void ModuleScene::RecieveEvent(const Event * e)
{
}
