#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"
#include "FileSystem.h"
#include "OutputLog.h"
#include "Texture2DManager.h"
#include "ShaderManager.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompMesh.h"
#include "RE_CompCamera.h"
#include "RE_CompParticleEmiter.h"
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

		ret = App->shaders->Load("particle", &shader_particle);
		if (!ret)
			LOG("%s\n", App->shaders->GetShaderError());
	}

	//smoke_particle = App->meshes->CreateMeshByTexture("Assets/Images/particle_texture.png");

	// root
	std::string path_scene("Assets/Scenes/");
	path_scene += GetName();
	path_scene += ".re";

	Config scene_file(path_scene.c_str(), App->fs->GetZipPath());
	if (scene_file.Load())
	{
		JSONNode* node = scene_file.GetRootNode("Game Objects");
		root = node->FillGO();
		DEL(node);

		// TODO: check if loaded scene already has camera
	}
	else
	{
		root = new RE_GameObject("root");
		// load default meshes
		//App->meshes->LoadMeshOnGameObject(root, "BakerHouse/BakerHouse.fbx");
		App->meshes->LoadMeshOnGameObject(root, "street/Street environment_V01.FBX");

		(new RE_GameObject("Main Camera", GUID_NULL, root))->AddComponent(C_CAMERA);
	}
	//root->SetBoundingBoxFromChilds();
	root->AddComponent(C_PLANE);

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


	// Spawn Firework on Key 1
	if (App->input->CheckKey(30))
	{
		RE_GameObject* smoke = App->scene->AddGO("Smoke");
		((RE_CompParticleEmitter*)smoke->AddComponent(C_PARTICLEEMITER))->SetUp(smoke_particle, shader_particle);
	}

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	Serialize();
	
	DEL(root);

	if (smoke_particle)
		DEL(smoke_particle);

	return true;
}

void ModuleScene::OnPlay()
{
	root->OnPlay();
}

void ModuleScene::OnPause()
{
	root->OnPause();
}

void ModuleScene::OnStop()
{
	root->OnStop();
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
	return new RE_GameObject(name, GUID_NULL, parent ? parent : root);
}

void ModuleScene::AddGoToRoot(RE_GameObject * toAdd)
{
	root->AddChild(toAdd);
}

void ModuleScene::DuplicateSelectedObject()
{
	if(selected != nullptr) selected->GetParent()->AddChild(new RE_GameObject(*selected));
}



void ModuleScene::DrawEditor()
{
	if (ImGui::CollapsingHeader(GetName()))
	{
		ImGui::Checkbox("Draw QuadTree", &draw_quad_tree);
		ImGui::Text("%i", drawn_go);
	}
}

void ModuleScene::DrawScene()
{
	if (draw_quad_tree)
		quad_tree.Draw();

	root->DrawAllAABB();

	//selected->DrawAABB();

	ShaderManager::use(modelloading); 
	ShaderManager::setFloat4x4(modelloading, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(modelloading, "projection", App->editor->GetCamera()->GetProjectionPtr());

	ShaderManager::setFloat4x4(shader_particle, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(shader_particle, "projection", App->editor->GetCamera()->GetProjectionPtr());
	
	// Frustum Culling
	std::vector<RE_GameObject*> objects;
	quad_tree.CollectIntersections(objects, App->editor->GetCamera()->GetFrustum());
	drawn_go = objects.size();

	//for (auto object : objects) object->Draw(false);

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

void ModuleScene::RayCastSelect(math::Ray & ray)
{
	std::vector<RE_GameObject*> objects;
	quad_tree.CollectIntersections(objects, ray);
	if (!objects.empty())
	{
		float closest_distance = -1.f;
		RE_GameObject* new_selection = nullptr;
		math::float4 camera_pos = math::float4(App->editor->GetCamera()->GetTransform()->GetGlobalPosition(), 0.f);

		for (auto object : objects)
		{
			RE_CompMesh* comp_mesh = object->GetMesh();
			if (comp_mesh != nullptr)
			{
				/*math::vec pos =	comp_mesh->GetClosestTriangleIntersectPos(ray, App->editor->GetCamera());
				float res_distance = math::Distance3(math::float4(pos, 0.f), camera_pos);
				if (closest_distance < 0.f || closest_distance > res_distance)
				{
					new_selection = object;
					closest_distance = res_distance;
				}*/
			}
		}
	}
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