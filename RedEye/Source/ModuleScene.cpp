#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"

#include "RE_FileSystem.h"
#include "RE_PrimitiveManager.h"
#include "RE_ResourceManager.h"
#include "RE_CameraManager.h"
#include "RE_ShaderImporter.h"
#include "RE_ModelImporter.h"
#include "RE_TextureImporter.h"

#include "RE_Material.h"
#include "RE_Prefab.h"

#include "RE_GameObject.h"
#include "RE_CompMesh.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CompPrimitive.h"

#include "OutputLog.h"
#include "RE_HandleErrors.h"
#include "TimeManager.h"

#include "md5.h"
#include <string>

#include "SDL2\include\SDL.h"


#define DEFAULTMODEL "Assets/Meshes/BakerHouse/BakerHouse.fbx"

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Init(JSONNode * node)
{
	defaultModel = node->PullString("defaultModel", DEFAULTMODEL);
	return true;
}

bool ModuleScene::Start()
{
	bool ret = true;

	// Load scene
	Timer timer;
	std::string path_scene("Assets/Scenes/");
	path_scene += GetName();
	path_scene += ".re";
	bool loadDefaultFBX = false;
	Config scene_file(path_scene.c_str(), App->fs->GetZipPath());
	if (scene_file.Load())
	{
		App->handlerrors->StartHandling();

		LOG("Importing scene from own format:");
		// Load saved scene
		JSONNode* node = scene_file.GetRootNode("Game Objects");
		RE_GameObject* loadedDO = node->FillGO();

		if (loadedDO) {
			root = loadedDO;
			DEL(node);
		}
		else {
			LOG_ERROR("Own serialized scene can't be loaded");
			loadDefaultFBX = true;
		}

		LOG("Time imported: %u ms", timer.Read());
	}
	else
		loadDefaultFBX = true;

	if(loadDefaultFBX)
	{
		App->handlerrors->StartHandling();

		// Load default FBX
		LOG("Importing scene from default asset model:");
		RE_Prefab* newModel = App->modelImporter->LoadModelFromAssets(defaultModel.c_str());
		if (newModel) {
			root = new RE_GameObject("root");
			root->AddChild(newModel->GetRoot());
			DEL(newModel);
		}
		else {
			LOG_ERROR("Cannot be lodadded the default model.\nAssetsPath: %s", defaultModel.c_str());
		}
	}

	// Error Handling
	App->handlerrors->StopHandling();
	if (App->handlerrors->AnyErrorHandled()) {
		App->handlerrors->ActivatePopUp();
	}

	// Grid Plane
	root->AddComponent(C_PLANE);

	// Render Camera Management
	App->cams->RecallCameras(root);
	if (!RE_CameraManager::HasMainCamera())
		CreateCamera();

	// Setup AABB + Quadtree
	root->TransformModified();
	UpdateQuadTree();

	return ret;
}

update_status ModuleScene::Update()
{
	OPTICK_CATEGORY("Update Scene", Optick::Category::GameLogic);

	root->Update();

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	Serialize();

	DEL(root);

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

void ModuleScene::RecieveEvent(const Event& e)
{
	switch (e.GetType())
	{
	case TRANSFORM_MODIFIED:
	{
		scene_modified = true;
		break;
	}
	case STATIC_TRANSFORM_MODIFIED:
	{
		static_gos_modified = true;
		break;
	}
	}
}

RE_GameObject * ModuleScene::GetRoot() const
{
	return root;
}

const RE_GameObject * ModuleScene::GetRoot_c() const
{
	return root;
}

RE_GameObject * ModuleScene::AddGO(const char * name, RE_GameObject * parent)
{
	return new RE_GameObject(name, GUID_NULL, parent ? parent : root);
}

void ModuleScene::AddGoToRoot(RE_GameObject * toAdd)
{
	root->AddChild(toAdd);
}

void ModuleScene::CreateCube()
{
	RE_GameObject* cube_go = AddGO("Cube", root);
	cube_go->AddComponent(App->primitives->CreateCube(cube_go));
}

void ModuleScene::CreateSphere()
{
	RE_GameObject* sphere_go = AddGO("Sphere", root);
	sphere_go->AddComponent(App->primitives->CreateSphere(sphere_go));
}

void ModuleScene::CreateCamera()
{
	RE_GameObject* cam_go = AddGO("Camera", root);
	cam_go->AddCompCamera();
}

void ModuleScene::DrawEditor()
{
	if (ImGui::CollapsingHeader(GetName()))
	{
		if (static_gos_modified && ImGui::Button("Reset AABB and Quadtree"))
			UpdateQuadTree();

		ImGui::Checkbox("Automatic Quadtree Update", &update_qt);

		int quadtree_drawing = quad_tree.GetDrawMode();
		if (ImGui::Combo("QuadTree Drawing", &quadtree_drawing, "Disable draw\0Top\0Bottom\0Top and Bottom\0All\0"))
			quad_tree.SetDrawMode(quadtree_drawing);
	}
}

void ModuleScene::DrawQTree() const
{
	quad_tree.Draw();
}

RE_GameObject* ModuleScene::RayCastSelect(math::Ray & ray)
{
	RE_GameObject* ret = nullptr;

	if (static_gos_modified && update_qt)
		UpdateQuadTree();

	std::vector<const RE_GameObject*> objects;
	if (static_gos_modified)
	{
		GetActive(objects);
	}
	else
	{
		GetActiveNonStatic(objects);
		quad_tree.CollectIntersections(objects, ray);
	}

	if (!objects.empty())
	{
		// Check Ray-AABB
		float res_distance;
		float res_distance2;
		std::vector<RE_CompMesh*> meshes;
		const math::float3 camera_pos = RE_CameraManager::CurrentCamera()->GetTransform()->GetGlobalPosition();

		for (auto object : objects)
		{
			RE_CompMesh* comp_mesh = object->GetMesh();
			if (comp_mesh != nullptr)
			{
				res_distance = 0.0f;
				res_distance2 = FLOAT_INF;
				if (object->GetGlobalBoundingBox().IntersectLineAABB_CPP(camera_pos, ray.dir, res_distance, res_distance2))
					meshes.push_back(comp_mesh);
			}
		}

		// Check Ray-Triangle
		RE_GameObject* new_selection = nullptr;
		if (!meshes.empty())
		{
			bool collision = false;
			float closest_distance = -1.f;
			for (auto mesh : meshes)
			{
				res_distance = 0.0f;
				if (mesh->CheckFaceCollision(ray, res_distance)
					&& (!collision || closest_distance > res_distance))
				{
					ret = mesh->GetGO();
					closest_distance = res_distance;
					collision = true;
				}
			}
		}
	}

	return ret;
}

void ModuleScene::FustrumCulling(std::vector<const RE_GameObject*>& container, math::Frustum & frustum)
{
	std::vector<const RE_GameObject*> objects;

	if (static_gos_modified && update_qt)
		UpdateQuadTree();

	if (static_gos_modified)
	{
		GetActive(objects);
	}
	else
	{
		GetActiveNonStatic(objects);
		quad_tree.CollectIntersections(container, frustum);
	}

	// Check Frustum-AABB
	for (auto object : objects)
		if (frustum.Intersects(object->GetGlobalBoundingBox()))
			container.push_back(object);
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
	root->SerializeJson(node);

	DEL(node);

	scene_file.Save();
}

void ModuleScene::LoadFBXOnScene(const char * fbxPath)
{
	std::string path(fbxPath);
	std::string fileName = path.substr(path.find_last_of("/") + 1);
	fileName = fileName.substr(0, fileName.find_last_of("."));

	std::string fbxOnLibrary("Library/Scenes/");
	fbxOnLibrary += fileName + ".efab";
	bool reloadFBX = false;
	RE_GameObject* toAdd = nullptr;
	if (App->fs->Exists(fbxOnLibrary.c_str())) {
		LOG_SECONDARY("Internal prefab of fbx exits. Loading from it.\nPath: %s", fbxOnLibrary.c_str());
		Config fbxPrefab(fbxOnLibrary.c_str(), App->fs->GetZipPath());
		if (fbxPrefab.Load()) {
			JSONNode* node = fbxPrefab.GetRootNode("Game Objects");
			toAdd = node->FillGO();
			DEL(node);
		}
		else {
			LOG_WARNING("Can't open internal prefab, reload from .fbx");
			reloadFBX = true;
		}
	}
	else
		reloadFBX = true;

	if(reloadFBX)
	{
		LOG_SECONDARY("Loading fbx on scene: %s", fbxPath);
		RE_Prefab* toLoad = App->modelImporter->LoadModelFromAssets(fbxPath);
		if (toLoad) {
			toAdd = toLoad->GetRoot();
			DEL(toLoad);
		}
		else {
			LOG_ERROR("Can't load the .fbx.\nAssetsPath: %s", fbxPath);
		}

	}

	if (toAdd)
	{
		root->AddChild(toAdd);
		root->TransformModified();
		root->ResetBoundingBoxFromChilds();
		quad_tree.Build(root);
		App->editor->SetSelected(toAdd, true);
	}
	else
		LOG_ERROR("Error to load dropped fbx");
}

void ModuleScene::LoadTextureOnSelectedGO(const char * texturePath)
{
	RE_GameObject* selected = App->editor->GetSelected();
	if (selected != nullptr)
	{
		RE_CompMesh* selectedMesh = selected->GetMesh();
		if (selectedMesh != nullptr) {
			std::string path(texturePath);
			std::string fileName = path.substr(path.find_last_of("/") + 1);
			fileName = fileName.substr(0, fileName.find_last_of("."));

			std::string md5Generated = md5(texturePath);
			const char* textureResource = App->resources->CheckFileLoaded(texturePath, md5Generated.c_str(), Resource_Type::R_TEXTURE);

			bool createMaterial = false;
			std::string filePath("Assets/Materials/");
			filePath += fileName;
			filePath += ".pupil";
			const char* materialMD5 = selectedMesh->GetMaterial();
			if (!materialMD5) {
				LOG_SECONDARY("Mesh missing Material. Creating new material: %s", filePath.c_str());
				if (!App->fs->Exists(filePath.c_str())) {
					createMaterial = true;
				}
				else {
					Config materialToLoad(filePath.c_str(), App->fs->GetZipPath());
					if (materialToLoad.Load()) {
						materialMD5 = App->resources->CheckFileLoaded(filePath.c_str(), materialToLoad.GetMd5().c_str(), Resource_Type::R_MATERIAL);
						selectedMesh->SetMaterial(materialMD5);
					}
					else {
						std::string newFile("Assets/Materials/");
						newFile += fileName;
						uint count = 0;

						do {
							count++;
							filePath = newFile;
							filePath += " ";
							filePath += std::to_string(count);
							filePath += ".pupil";
						} while (!App->fs->Exists(filePath.c_str()));

						createMaterial = true;
					}
				}

			if (createMaterial) {
				LOG_SECONDARY("Creating new material: %s", filePath.c_str());
				RE_Material* newMaterial = new RE_Material();

					newMaterial->tDiffuse.push_back(textureResource);

				((ResourceContainer*)newMaterial)->SetName(fileName.c_str());
				((ResourceContainer*)newMaterial)->SetAssetPath(filePath.c_str());
				((ResourceContainer*)newMaterial)->SetType(Resource_Type::R_MATERIAL);
				newMaterial->Save();

					materialMD5 = App->resources->Reference((ResourceContainer*)newMaterial);
					selectedMesh->SetMaterial(materialMD5);
				}
			}
			else {
				LOG_SECONDARY("Material on mesh found, changing texture and saving.");

				RE_Material* selectedMaterial = (RE_Material*)App->resources->At(materialMD5);
				if (selectedMaterial) {
					if (selectedMaterial->tDiffuse.empty())
						selectedMaterial->tDiffuse.push_back(textureResource);
					else
						selectedMaterial->tDiffuse[0] = textureResource;

					selectedMaterial->Save();
				}
			}
		}
		else
			LOG_ERROR("Selected GameObject does not have a mesh");
	}
	else
		LOG_ERROR("No Selected GameObject");
}

void ModuleScene::GetActive(std::vector<const RE_GameObject*>& objects) const
{
	std::queue<const RE_GameObject*> queue;

	for (auto child : root->GetChilds())
		if (child->IsActive())
			queue.push(child);

	while (!queue.empty())
	{
		const RE_GameObject* obj = queue.front();
		objects.push_back(obj);

		for (auto child : obj->GetChilds())
			if (child->IsActive())
				queue.push(child);

		queue.pop();
	}
}

void ModuleScene::GetActiveStatic(std::vector<const RE_GameObject*>& objects) const
{
	std::queue<const RE_GameObject*> queue;

	for (auto child : root->GetChilds())
		if (child->IsActive() && child->IsStatic())
			queue.push(child);

	while (!queue.empty())
	{
		const RE_GameObject* obj = queue.front();
		objects.push_back(obj);

		for (auto child : obj->GetChilds())
			if (child->IsActive() && child->IsStatic())
				queue.push(child);

		queue.pop();
	}
}

void ModuleScene::GetActiveNonStatic(std::vector<const RE_GameObject*>& objects) const
{
	std::queue<const RE_GameObject*> queue;

	for (auto child : root->GetChilds())
		if (child->IsActive() && !child->IsStatic())
			queue.push(child);

	while (!queue.empty())
	{
		const RE_GameObject* obj = queue.front();
		objects.push_back(obj);

		for (auto child : obj->GetChilds())
			if (child->IsActive() && !child->IsStatic())
				queue.push(child);

		queue.pop();
	}
}

void ModuleScene::UpdateQuadTree()
{
	root->ResetBoundingBoxFromChilds();
	quad_tree.Build(root);
	static_gos_modified = false;
}
