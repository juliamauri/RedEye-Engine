#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"

#include "RE_FileSystem.h"
#include "RE_PrimitiveManager.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_CameraManager.h"
#include "RE_ShaderImporter.h"
#include "RE_ModelImporter.h"
#include "RE_TextureImporter.h"

#include "RE_Material.h"
#include "RE_Prefab.h"
#include "RE_Scene.h"
#include "RE_Model.h"

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
#include <queue>

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

	root = new RE_GameObject("root");
	root->SetStatic(false, false);

	// Load scene
	Timer timer;
	std::string path_scene("Assets/Scenes/");
	path_scene += GetName();
	path_scene += ".re";

	App->internalResources->FindDefaultSkyBox();

	sceneLoadedMD5 = App->resources->FindMD5ByAssetsPath(path_scene.c_str(), Resource_Type::R_SCENE);

	bool loadDefaultFBX = false;

	if (sceneLoadedMD5 != nullptr)
	{
		LOG("Importing scene from own format:");

		App->handlerrors->StartHandling();

		RE_Scene* scene = (RE_Scene * )App->resources->At(sceneLoadedMD5);
		Event::PauseEvents();
		RE_GameObject* loadedDO = scene->GetRoot();

		if (loadedDO)
			root = loadedDO;
		else {
			LOG_ERROR("Own serialized scene can't be loaded");
			loadDefaultFBX = true;
		}

		LOG("Time imported scene: %u ms", timer.Read());
	}
	else
		loadDefaultFBX = true;

	if(loadDefaultFBX)
	{
		const char* defaultModelMD5 = App->resources->FindMD5ByAssetsPath(defaultModel.c_str(), Resource_Type::R_MODEL);

		if (defaultModelMD5) {
			App->handlerrors->StartHandling();

			RE_Model* model = (RE_Model*)App->resources->At(defaultModelMD5);

			Event::PauseEvents();
			RE_GameObject* modelCopy = model->GetRoot();
			Event::ResumeEvents();

			if (modelCopy) {
				root = new RE_GameObject("root");
				root->AddChild(modelCopy);
			}
			else {
				LOG_ERROR("Cannot be lodadded the default model.\nAssetsPath: %s", defaultModel.c_str());
			}

		}
		else {
			LOG_ERROR("Default Model can't be loaded");
		}
		LOG("Time importing main model: %u ms", timer.Read());
	}

	// Error Handling
	App->handlerrors->StopHandling();
	if (App->handlerrors->AnyErrorHandled()) {
		App->handlerrors->ActivatePopUp();
	}

	// Render Camera Management
	App->cams->RecallCameras(root);
	if (!RE_CameraManager::HasMainCamera())
		CreateCamera();

	root->UseResources();
	// Setup AABB + Quadtree
	Event::PauseEvents();
	UpdateQuadTree();
	Event::ResumeEvents();

	if (root)
	{
		//GO Manager!!!
		std::map< RE_GameObject*, int> gosID = goManager.PushWithChilds(root);
		Event::PauseEvents();
		savedState = new RE_GameObject(*root);
		Event::ResumeEvents();

		// Render Camera Management
		App->cams->RecallCameras(root);
		if (!RE_CameraManager::HasMainCamera())
			CreateCamera();

		// Setup AABB + Quadtree
		root->TransformModified(false);
		UpdateQuadTree();
	}

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
	DEL(savedState);

	return true;
}

void ModuleScene::OnPlay()
{
	Event::PauseEvents();
	if (savedState) DEL(savedState);
	savedState = new RE_GameObject(*root);
	Event::ResumeEvents(); 
	root->OnPlay();
}

void ModuleScene::OnPause()
{
	root->OnPause();
}

void ModuleScene::OnStop()
{
	root->OnStop();
	DEL(root);
	root = new RE_GameObject(*savedState);
	App->editor->SetSelected(nullptr);

	// Render Camera Management
	App->cams->RecallCameras(root);
	if (!RE_CameraManager::HasMainCamera())
		CreateCamera();

	// Setup AABB + Quadtree
	root->TransformModified(true);
	GetActiveStatic(active_static_gos);
	GetActiveNonStatic(active_non_static_gos);
	UpdateQuadTree();
}

void ModuleScene::RecieveEvent(const Event& e)
{
	RE_GameObject* go = e.data1.AsGO();
	if (go != nullptr)
	{
		bool belongs_to_scene = false;

		for (const RE_GameObject* parent = go;
			parent != nullptr && !belongs_to_scene;
			parent = parent->GetParent_c())
		{
			if (belongs_to_scene = (parent == root))
				break;
		}

		if (belongs_to_scene)
			scene_modified = belongs_to_scene;

		switch (e.type)
		{
		case GO_CHANGED_TO_ACTIVE:
		{
			if (belongs_to_scene)
			{
				if (go->GetParent())
					go->GetParent()->ResetBoundingBoxFromChilds();
				else
					go->ResetGlobalBoundingBox();

				if (go->IsStatic())
				{
					static_gos_modified = true;

					std::list<RE_GameObject*> added_gos;
					if (quad_tree.TryPushingWithChilds(go, added_gos))
					{
						for (auto added_go : added_gos)
						{
							active_static_gos.push_back(added_go);
							tree_free_static_gos.remove(added_go);
						}
					}
					else
					{
						std::queue<RE_GameObject*> queue;
						queue.push(go);
						while (!queue.empty())
						{
							RE_GameObject* obj = queue.front();
							queue.pop();

							if (obj->IsActive())
							{
								if (obj->IsStatic())
								{
									active_static_gos.push_back(obj);
									tree_free_static_gos.push_back(obj);
								}
								else
									active_static_gos.push_back(obj);
							}

							for (auto child : obj->GetChilds())
								queue.push(child);
						}

						if (update_qt)
							UpdateQuadTree();
					}
				}
				else
				{
					std::queue<RE_GameObject*> queue;
					queue.push(go);
					while (!queue.empty())
					{
						RE_GameObject* obj = queue.front();
						queue.pop();

						if (obj->IsActive())
							active_non_static_gos.push_back(obj);

						for (auto child : obj->GetChilds())
							queue.push(child);
					}
				}
			}
			else if (go->GetParent() != nullptr)
				go->GetParent()->ResetBoundingBoxFromChilds();
			else
				go->ResetGlobalBoundingBox();

			break;
		}
		case GO_CHANGED_TO_INACTIVE:
		{
			if (belongs_to_scene)
			{
				std::queue<RE_GameObject*> queue;
				queue.push(go);
				while (!queue.empty())
				{
					RE_GameObject* obj = queue.front();
					queue.pop();

					if (obj->IsStatic())
					{
						if (quad_tree.Contains(obj->GetGlobalBoundingBox()))
						{
							quad_tree.Pop(obj);
							active_static_gos.remove(obj);
						}
						else
							tree_free_static_gos.remove(obj);
					}
					else
						active_non_static_gos.remove(obj);

					for (auto child : obj->GetChilds())
						queue.push(child);
				}
			}
			break;
		}
		case GO_CHANGED_TO_STATIC:
		{
			if (belongs_to_scene)
			{
				static_gos_modified = true;

				std::list<RE_GameObject*> added_gos;
				if (quad_tree.TryPushingWithChilds(go, added_gos))
				{
					for (auto added_go : added_gos)
					{
						active_static_gos.push_back(added_go);
						active_non_static_gos.remove(added_go);
					}
				}
				else
				{
					std::queue<RE_GameObject*> queue;
					queue.push(go);
					while (!queue.empty())
					{
						RE_GameObject* obj = queue.front();
						queue.pop();

						active_static_gos.push_back(obj);
						tree_free_static_gos.push_back(obj);
						active_non_static_gos.remove(obj);

						for (auto child : obj->GetChilds())
							if (!go->IsActive())
								queue.push(child);
					}

					if (update_qt)
						UpdateQuadTree();
				}
			}
			break;
		}
		case GO_CHANGED_TO_NON_STATIC:
		{
			if (belongs_to_scene)
			{
				std::queue<RE_GameObject*> queue;
				queue.push(go);
				while (!queue.empty())
				{
					RE_GameObject* obj = queue.front();
					queue.pop();

					if (quad_tree.Contains(obj->GetGlobalBoundingBox()))
						quad_tree.Pop(obj);
					else
						tree_free_static_gos.remove(obj);

					active_static_gos.remove(obj);
					active_non_static_gos.push_back(obj);

					for (auto child : obj->GetChilds())
						queue.push(child);
				}
			}
			break;
		}
		case GO_HAS_NEW_CHILD:
		{
			RE_GameObject* to_add = e.data2.AsGO();

			if (belongs_to_scene)
			{
				go->ResetBoundingBoxFromChilds();

				if (to_add->IsStatic())
				{
					static_gos_modified = true;

					std::list<RE_GameObject*> added_gos;
					if (quad_tree.TryAdapting(go) &&
						quad_tree.TryPushingWithChilds(to_add, added_gos))
					{
						for (auto added_go : added_gos)
							active_static_gos.push_back(added_go);
					}
					else
					{
						std::queue<RE_GameObject*> queue;
						queue.push(go);
						while (!queue.empty())
						{
							RE_GameObject* obj = queue.front();
							queue.pop();

							if (obj->IsActive())
							{
								active_static_gos.push_back(obj);
								tree_free_static_gos.push_back(obj);
							}

							for (auto child : obj->GetChilds())
								queue.push(child);
						}

						if (update_qt)
							UpdateQuadTree();
					}
				}
				else
				{
					std::queue<RE_GameObject*> queue;
					queue.push(go);
					while (!queue.empty())
					{
						RE_GameObject* obj = queue.front();
						queue.pop();

						if (obj->IsActive())
						{
							if (obj->IsStatic())
								active_static_gos.push_back(obj);
							else
								active_non_static_gos.push_back(obj);
						}

						for (auto child : obj->GetChilds())
							queue.push(child);
					}
				}
			}
			else if (go->GetParent() != nullptr)
				go->GetParent()->ResetBoundingBoxFromChilds();
			else
				go->ResetGlobalBoundingBox();
			break;
		}
		case GO_HAS_NEW_CHILDS:
		{
			if (belongs_to_scene)
			{
				active_static_gos.clear();
				active_non_static_gos.clear();
				tree_free_static_gos.clear();

				std::queue<RE_GameObject*> queue;
				for (auto childs : root->GetChilds())
					queue.push(childs);

				while (!queue.empty())
				{
					RE_GameObject* obj = queue.front();
					queue.pop();

					if (obj->IsActive())
					{
						if (obj->IsStatic())
							active_static_gos.push_back(obj);
						else
							active_non_static_gos.push_back(obj);
					}

					for (auto child : obj->GetChilds())
						queue.push(child);
				}

				UpdateQuadTree();
			}
			else if (go->GetParent() != nullptr)
				go->GetParent()->ResetBoundingBoxFromChilds();
			else
				go->ResetGlobalBoundingBox();
			break;
		}
		case GO_REMOVE_CHILD:
		{
			RE_GameObject* to_remove = e.data2.AsGO();
			if (belongs_to_scene)
			{
				std::queue<RE_GameObject*> queue;
				queue.push(to_remove);
				while (!queue.empty())
				{
					RE_GameObject* obj = queue.front();
					queue.pop();

					if (obj->IsStatic())
					{
						if (quad_tree.Contains(obj->GetGlobalBoundingBox()))
						{
							quad_tree.Pop(obj);
							active_static_gos.remove(obj);
						}
						else
							tree_free_static_gos.remove(obj);
					}
					else
						active_non_static_gos.remove(obj);

					for (auto child : obj->GetChilds())
						queue.push(child);
				}
			}

			go->GetChilds().remove(to_remove);

			break;
		}
		case TRANSFORM_MODIFIED:
		{
			for (auto child : go->GetChilds())
				child->TransformModified(true);

			if (belongs_to_scene)
			{
				if (go->IsStatic())
				{
					static_gos_modified = true;
					std::list<RE_GameObject*> adapted_gos;
					if (!quad_tree.TryAdaptingWithChilds(go, adapted_gos))
					{
						if (update_qt)
							UpdateQuadTree();
						else
						{
							bool was_out_of_qtree = false;
							std::list<RE_GameObject*>::iterator it = tree_free_static_gos.begin();
							for (; it != tree_free_static_gos.end(); it++)
								if (was_out_of_qtree = (it._Ptr->_Myval == go))
									break;

							if (!was_out_of_qtree)
							{
								std::queue<RE_GameObject*> queue;
								queue.push(go);
								while (!queue.empty())
								{
									RE_GameObject* obj = queue.front();
									queue.pop();

									quad_tree.Pop(obj);
									tree_free_static_gos.push_back(obj);

									for (auto child : obj->GetChilds())
										queue.push(child);
								}
							}
						}
					}
					else
					{
						for (auto adapted_go : adapted_gos)
							tree_free_static_gos.remove(adapted_go);
					}
				}
			}

			break;
		}
		case PLANE_CHANGE_TO_MESH:
		{
			RE_CompPlane* plane = (RE_CompPlane * )go->GetComponent(C_PLANE);
			const char* planeMD5 = plane->TransformAsMeshResource();
			go->RemoveComponent(plane);
			RE_CompMesh* newMesh = new RE_CompMesh(go, planeMD5);
			newMesh->UseResources();
			go->AddCompMesh(newMesh);
			break;
		}
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

RE_GameObject * ModuleScene::AddGO(const char * name, RE_GameObject * parent, bool broadcast)
{
	RE_GameObject* ret = new RE_GameObject(name, GUID_NULL, parent ? parent : root);

	return ret;
}

void ModuleScene::AddGoToRoot(RE_GameObject * toAdd)
{
	root->AddChild(toAdd);
}

void ModuleScene::CreatePlane()
{
	RE_GameObject* plane_go = AddGO("Plane", root);
	plane_go->AddComponent(App->primitives->CreatePlane(plane_go));
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
		ImGui::Text("Total active: %i", active_non_static_gos.size() + active_static_gos.size());
		ImGui::Text(" - Non Static: %i", active_non_static_gos.size());
		ImGui::Text(" - Static: %i", active_static_gos.size());
		ImGui::Text(" - Static outside QTree: %i", tree_free_static_gos.size());

		if (!update_qt && static_gos_modified && ImGui::Button("Reset AABB and Quadtree"))
			UpdateQuadTree();

		//ImGui::Checkbox("Automatic Quadtree Update", &update_qt);

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
	std::vector<RE_GameObject*> objects;

	// Add non static
	for (auto go : active_non_static_gos)
		objects.push_back(go);

	// Add static
	quad_tree.CollectIntersections(objects, ray);
	for (auto go : tree_free_static_gos)
		objects.push_back(go);

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
	std::vector<RE_GameObject*> objects;

	// Add non static
	for (auto go : active_non_static_gos)
		objects.push_back(go);

	// Add static
	quad_tree.CollectIntersections(objects, frustum);
	for (auto go : tree_free_static_gos)
		objects.push_back(go);

	// Check Frustum-AABB
	for (auto object : objects)
		if (frustum.Intersects(object->GetGlobalBoundingBox()))
			container.push_back(object);
}

void ModuleScene::Serialize()
{
	RE_Scene* scene = nullptr;
	if (sceneLoadedMD5 == nullptr) {
		scene = new RE_Scene();
		scene->SetName(GetName());
		scene->SetType(Resource_Type::R_SCENE);
	}
	else
		scene = (RE_Scene * )App->resources->At(sceneLoadedMD5);

	scene->Save(root);
	scene->SaveMeta();

	if (sceneLoadedMD5 == nullptr) {
		sceneLoadedMD5 = App->resources->Reference(scene);
	}
}

void ModuleScene::LoadFBXOnScene(const char * fbxPath)
{
	//std::string path(fbxPath);
	//std::string fileName = path.substr(path.find_last_of("/") + 1);
	//fileName = fileName.substr(0, fileName.find_last_of("."));

	//std::string fbxOnLibrary("Library/Scenes/");
	//fbxOnLibrary += fileName + ".efab";
	//bool reloadFBX = false;
	//RE_GameObject* toAdd = nullptr;
	//if (App->fs->Exists(fbxOnLibrary.c_str())) {
	//	LOG_SECONDARY("Internal prefab of fbx exits. Loading from it.\nPath: %s", fbxOnLibrary.c_str());
	//	Config fbxPrefab(fbxOnLibrary.c_str(), App->fs->GetZipPath());
	//	if (fbxPrefab.Load()) {
	//		JSONNode* node = fbxPrefab.GetRootNode("Game Objects");
	//		toAdd = node->FillGO();
	//		DEL(node);
	//	}
	//	else {
	//		LOG_WARNING("Can't open internal prefab, reload from .fbx");
	//		reloadFBX = true;
	//	}
	//}
	//else
	//	reloadFBX = true;

	//if(reloadFBX)
	//{
	//	LOG_SECONDARY("Loading fbx on scene: %s", fbxPath);
	//	RE_Prefab* toLoad = App->modelImporter->LoadModelFromAssets(fbxPath);
	//	if (toLoad) {
	//		toAdd = toLoad->GetRoot();
	//		DEL(toLoad);
	//	}
	//	else {
	//		LOG_ERROR("Can't load the .fbx.\nAssetsPath: %s", fbxPath);
	//	}

	//}

	//if (toAdd)
	//{
	//	root->AddChild(toAdd);
	//	root->TransformModified();
	//	root->ResetBoundingBoxFromChilds();
	//	quad_tree.Build(root);
	//	App->editor->SetSelected(toAdd, true);
	//}
	//else
	//	LOG_ERROR("Error to load dropped fbx");
}

void ModuleScene::LoadTextureOnSelectedGO(const char * texturePath)
{
	//RE_GameObject* selected = App->editor->GetSelected();
	//if (selected != nullptr)
	//{
	//	RE_CompMesh* selectedMesh = selected->GetMesh();
	//	if (selectedMesh != nullptr) {
	//		std::string path(texturePath);
	//		std::string fileName = path.substr(path.find_last_of("/") + 1);
	//		fileName = fileName.substr(0, fileName.find_last_of("."));

	//		std::string md5Generated = md5(texturePath);
	//		const char* textureResource = App->resources->CheckFileLoaded(texturePath, md5Generated.c_str(), Resource_Type::R_TEXTURE);

	//		bool createMaterial = false;
	//		std::string filePath("Assets/Materials/");
	//		filePath += fileName;
	//		filePath += ".pupil";
	//		const char* materialMD5 = selectedMesh->GetMaterial();
	//		if (!materialMD5) {
	//			LOG_SECONDARY("Mesh missing Material. Creating new material: %s", filePath.c_str());
	//			if (!App->fs->Exists(filePath.c_str())) {
	//				createMaterial = true;
	//			}
	//			else {
	//				Config materialToLoad(filePath.c_str(), App->fs->GetZipPath());
	//				if (materialToLoad.Load()) {
	//					materialMD5 = App->resources->IsReference(materialToLoad.GetMd5().c_str(), Resource_Type::R_MATERIAL);
	//					selectedMesh->SetMaterial(materialMD5);
	//				}
	//				else {
	//					std::string newFile("Assets/Materials/");
	//					newFile += fileName;
	//					uint count = 0;

	//					do {
	//						count++;
	//						filePath = newFile;
	//						filePath += " ";
	//						filePath += std::to_string(count);
	//						filePath += ".pupil";
	//					} while (!App->fs->Exists(filePath.c_str()));

	//					createMaterial = true;
	//				}
	//			}

	//		if (createMaterial) {
	//			LOG_SECONDARY("Creating new material: %s", filePath.c_str());
	//			RE_Material* newMaterial = new RE_Material();

	//				newMaterial->tDiffuse.push_back(textureResource);

	//			((ResourceContainer*)newMaterial)->SetName(fileName.c_str());
	//			((ResourceContainer*)newMaterial)->SetAssetPath(filePath.c_str());
	//			((ResourceContainer*)newMaterial)->SetType(Resource_Type::R_MATERIAL);
	//			newMaterial->Save();

	//				materialMD5 = App->resources->Reference((ResourceContainer*)newMaterial);
	//				selectedMesh->SetMaterial(materialMD5);
	//			}
	//		}
	//		else {
	//			LOG_SECONDARY("Material on mesh found, changing texture and saving.");

	//			RE_Material* selectedMaterial = (RE_Material*)App->resources->At(materialMD5);
	//			if (selectedMaterial) {
	//				if (selectedMaterial->tDiffuse.empty())
	//					selectedMaterial->tDiffuse.push_back(textureResource);
	//				else
	//					selectedMaterial->tDiffuse[0] = textureResource;

	//				selectedMaterial->Save();
	//			}
	//		}
	//	}
	//	else
	//		LOG_ERROR("Selected GameObject does not have a mesh");
	//}
	//else
	//	LOG_ERROR("No Selected GameObject");
}

void ModuleScene::GetActive(std::list<RE_GameObject*>& objects) const
{
	std::queue<RE_GameObject*> queue;

	for (auto child : root->GetChilds())
		if (child->IsActive())
			queue.push(child);

	while (!queue.empty())
	{
		RE_GameObject* obj = queue.front();
		objects.push_back(obj);

		for (auto child : obj->GetChilds())
			if (child->IsActive())
				queue.push(child);

		queue.pop();
	}
}

void ModuleScene::GetActiveStatic(std::list<RE_GameObject*>& objects) const
{
	std::queue<RE_GameObject*> queue;

	for (auto child : root->GetChilds())
		if (child->IsActive() && child->IsStatic())
			queue.push(child);

	while (!queue.empty())
	{
		RE_GameObject* obj = queue.front();
		objects.push_back(obj);

		for (auto child : obj->GetChilds())
			if (child->IsActive() && child->IsStatic())
				queue.push(child);

		queue.pop();
	}
}

void ModuleScene::GetActiveNonStatic(std::list<RE_GameObject*>& objects) const
{
	std::queue<RE_GameObject*> queue;

	for (auto child : root->GetChilds())
		if (child->IsActive() && !child->IsStatic())
			queue.push(child);

	while (!queue.empty())
	{
		RE_GameObject* obj = queue.front();
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

	while(!tree_free_static_gos.empty())
	{
		active_static_gos.push_front(tree_free_static_gos.back());
		tree_free_static_gos.pop_back();
	}

	quad_tree.Build(root);
	static_gos_modified = false;
}

std::map< RE_GameObject*, int> GameObjectManager::PushWithChilds(RE_GameObject* val, bool root)
{
	std::map< RE_GameObject*, int> ret;
	 std::vector<RE_GameObject*> gos =  val->GetAllGO();
	 if (!root) gos.erase(gos.begin());
	 for (auto go : gos) ret.insert(std::pair<RE_GameObject*,int>(go, Push(go)));
	 return ret;
}

int GameObjectManager::Push(RE_GameObject* val)
{
	int ret = lastAvaibleIndex;
	PoolMapped::Push(val, ret);
	return ret;
}