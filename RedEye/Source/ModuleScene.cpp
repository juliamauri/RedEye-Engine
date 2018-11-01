#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "FileSystem.h"
#include "OutputLog.h"
#include "Texture2DManager.h"
#include "ShaderManager.h"
//#include "RE_CompMesh.h"
#include "RE_Mesh.h"
#include "RE_Camera.h"
#include "RE_PrimitiveManager.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include <string>
#include <algorithm>

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Start()
{
	bool ret = true;

	//Loading Shaders
	if (App->shaders)
	{
		ret = App->shaders->Load("texture", &modelloading);
		if (!ret)
			LOG("%s\n", App->shaders->GetShaderError());
	}

	root = new RE_GameObject();
	root->AddComponent(C_PLANE);
	
	return ret;
}

update_status ModuleScene::Update()
{
	root->Update();
	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	DEL(root);
	return true;
}

void ModuleScene::FileDrop(const char * file)
{
	RE_FileIO* holder = App->fs->QuickBufferFromPDPath(file);

	std::string full_path(file);
	std::string file_name = full_path.substr(full_path.find_last_of("\\") + 1);
	std::string directory = full_path.substr(0, full_path.find_last_of('\\'));
	std::string ext = full_path.substr(full_path.find_last_of(".") + 1);

	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext.compare("fbx") == 0)
	{
		/*RE_CompMesh* c_mesh = (RE_CompMesh*)drop->GetComponent(C_MESH);

		if (c_mesh == nullptr)
			drop->AddComponent(C_MESH, (char*)file, true);
		else
			c_mesh->LoadMesh(file, true);*/

		// FOCUS ON DROP
		/*DEL(mesh_droped);

		drop->GetComponent(C_TRANSFORM)->Reset();

		mesh_droped = new RE_CompUnregisteredMesh((char*)file, holder->GetBuffer(), holder->GetSize());

		App->renderer3d->camera->SetPosition(mesh_droped->bounding_box.maxPoint * 2);
		App->renderer3d->camera->SetFocus(mesh_droped->bounding_box.CenterPoint());*/
	}
	else if (ext.compare("jpg") == 0 || ext.compare("png") == 0 || ext.compare("dds") == 0)
	{
		App->textures->LoadTexture2D(directory.c_str(), file_name.c_str(), true);
	}

	DEL(holder);
}

void ModuleScene::RecieveEvent(const Event * e)
{
}

void ModuleScene::DrawScene()
{
	root->Draw();

	// mesh drawing
	/*if (mesh_droped)
	{
		// glm calls
		ShaderManager::use(modelloading);
		ShaderManager::setFloat4x4(modelloading, "model", drop->GetTransform()->GetGlobalMatrix().ptr());
		ShaderManager::setFloat4x4(modelloading, "view", App->renderer3d->camera->GetView().ptr());
		ShaderManager::setFloat4x4(modelloading, "projection", App->renderer3d->camera->GetProjection().ptr());
		mesh_droped->Draw(modelloading);

		// mathgeolib calls
		ShaderManager::use(modelloading);
		ShaderManager::setFloat4x4(modelloading, "model", root->transform->GetGlobalMatrix().ptr());
		ShaderManager::setFloat4x4(modelloading, "view", App->renderer3d->camera->GetView().ptr());
		ShaderManager::setFloat4x4(modelloading, "projection", App->renderer3d->camera->GetProjection().ptr());
		ShaderManager::setFloat(modelloading, "viewPos", math::vec(.0f, 0.0f, 10.0f));
		math::float3x3 modelNormal(root->transform->GetGlobalMatrix().InverseTransposed().Float3x3Part());
		ShaderManager::setFloat3x3(modelloading, "modelNormal", modelNormal.ptr());
		mesh_droped->Draw(modelloading);
	}*/
	

	//drop->Draw();
}

void ModuleScene::DrawFocusedProperties()
{
	// DRAW SELECTED GO
	/*if (mesh_droped)
	{
		// root->DrawProperties();

		RE_CompTransform* transform = (RE_CompTransform*)drop->GetComponent(C_TRANSFORM);
		transform->DrawProperties();
		mesh_droped->DrawProperties();
	}*/
}
