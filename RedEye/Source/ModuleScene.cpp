#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "FileSystem.h"
#include "OutputLog.h"
#include "Texture2DManager.h"
#include "ShaderManager.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompMesh.h"
#include "RE_CompCamera.h"
#include "MeshManager.h"
#include <string>
#include <algorithm>

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled) {}

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
		App->meshes->SetDefaultShader(modelloading);
	}

	// root
	root = new RE_GameObject("root");
	root->AddComponent(C_PLANE);

	// load default meshes
	App->meshes->LoadMeshOnGameObject(root, "street/Street environment_V01.FBX");
	//App->meshes->LoadMeshOnGameObject(root, "BakerHouse/BakerHouse.fbx");

	if (!root->GetChilds().empty())
	{
		selected = *root->GetChilds().begin();
		selected->SetBoundingBoxFromChilds();
		root->SetBoundingBoxFromChilds();
	}
	else
		selected = root;


	//selected->SetBoundingBox(math::AABB(math::Sphere({ 0.0f, 0.0f, 0.0f }, 1.0f)));

	//// depricated way
	//selected = new RE_GameObject("Street", root);
	//selected->AddCompMesh("path");
	//selected->SetBoundingBox(math::AABB(math::Sphere({ 0.0f, 0.0f, 0.0f }, 1.0f)));



	// call this instead to create all gameobjects from fbx
	//selected = App->meshes->DumpGeometry(root, "path del street fbx");

	quad_tree.Build(root);

	return ret;
}

update_status ModuleScene::Update()
{
	root->Update();
	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	Serialize();
	
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
		// CREATE GAMEOBJECTS WITH GEOMETRY
		if (selected != nullptr) selected = root;

		//selected = App->meshes->DumpGeometry(selected, "path del street fbx");

		// FOCUS CAMERA ON DROPPED GEOMETRY
		//App->editor->GetCamera()->SetPosition(selected->GetBoundingBox().maxPoint * 2);
		//App->editor->GetCamera()->SetFocus(selected->GetBoundingBox().CenterPoint());
	}
	else if (ext.compare("jpg") == 0 || ext.compare("png") == 0 || ext.compare("dds") == 0)
	{
		App->textures->LoadTexture2D(directory.c_str(), file_name.c_str(), true);
	}

	DEL(holder);
}

void ModuleScene::RecieveEvent(const Event& e)
{
}

RE_GameObject * ModuleScene::AddGO(const char * name, RE_GameObject * parent)
{
	RE_GameObject* ret = new RE_GameObject(name, GUID_NULL, parent ? parent : root);

	return ret;
}

void ModuleScene::DrawEditor()
{
	if (ImGui::CollapsingHeader(GetName()))
	{
		ImGui::Checkbox("Draw QuadTree", &draw_quad_tree);
	}
}

void ModuleScene::DrawScene()
{
	if (draw_quad_tree)
		quad_tree.Draw();

	//root->DrawAllAABB();

	selected->DrawAABB();

	ShaderManager::use(modelloading); 
	ShaderManager::setFloat4x4(modelloading, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(modelloading, "projection", App->editor->GetCamera()->GetProjectionPtr());

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
}

void ModuleScene::DrawHeriarchy()
{
	if (root != nullptr)
	{
		for (auto child : root->GetChilds())
			child->DrawHeriarchy();
	}
}

void ModuleScene::DrawFocusedProperties()
{
	// DRAW SELECTED GO
	if (selected != nullptr && selected != root)
	{
		selected->DrawProperties();
	}
}

void ModuleScene::SetSelected(RE_GameObject * select)
{
	selected = (select != nullptr) ? select : root;
}

RE_GameObject * ModuleScene::GetSelected() const
{
	return selected;
}

void ModuleScene::Serialize()
{
	char* buffer = nullptr;
	
	std::string path_scene("Assets/Scenes/");
	path_scene += GetName();
	path_scene += ".re";

	Config scene_file(path_scene.c_str(), App->fs->GetZipPath());

	JSONNode* node = scene_file.GetRootNode("Game Objects");

	node->SetArray();
	root->Serialize(node);

	DEL(node);

	scene_file.Save();
}
