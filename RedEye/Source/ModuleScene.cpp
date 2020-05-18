#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"
#include "RE_ThumbnailManager.h"

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
#include <EASTL/string.h>
#include <EASTL/queue.h>
#include <EASTL/vector.h>

#include "SDL2\include\SDL.h"


#define DEFAULTMODEL "Assets/Meshes/BakerHouse/BakerHouse.fbx"

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Init(JSONNode * node)
{
	return true;
}

bool ModuleScene::Start()
{
	bool ret = true;

	eastl::vector<ResourceContainer*> scenes = App->resources->GetResourcesByType(Resource_Type::R_SCENE);
	if (!scenes.empty())
		LoadScene(scenes[0]->GetMD5());
	else
		NewEmptyScene();

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
	DEL(root);
	DEL(savedState);
	if (unsavedScene) DEL(unsavedScene);

	return true;
}

void ModuleScene::OnPlay()
{
	Event::PauseEvents();
	DEL(savedState);
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

	Event::PauseEvents();

	DEL(root);
	root = new RE_GameObject(*savedState);
	App->editor->SetSelected(nullptr);
	goManager.Clear();

	SetupScene();
	Event::ResumeEvents();
}

void ModuleScene::RecieveEvent(const Event& e)
{
	RE_GameObject* go = e.data1.AsGO();
	if (go != nullptr)
	{
		bool belongs_to_scene = false;// = (go->root == root);

		for (const RE_GameObject* parent = go;
			parent != nullptr && !belongs_to_scene;
			parent = parent->GetParent_c())
		{
			if (parent == root)
			{
				belongs_to_scene = true;
				break;
			}
		}

		switch (e.type)
		{
		case GO_CHANGED_TO_ACTIVE:
		{
			eastl::vector<RE_GameObject*> all = go->GetActiveChildsWithDrawComponents();

			if (belongs_to_scene)
			{
				for (auto draw_go : all)
				{
					draw_go->ResetGlobalBoundingBox();

					if (draw_go->IsStatic())
						static_tree.PushNode(goManager.WhatID(draw_go), draw_go->GetGlobalBoundingBox());
					else
						dynamic_tree.PushNode(goManager.WhatID(draw_go), draw_go->GetGlobalBoundingBox());
				}
			}
			else
				for (auto draw_go : all)
					draw_go->ResetGlobalBoundingBox();

			break;
		}
		case GO_CHANGED_TO_INACTIVE:
		{
			if (belongs_to_scene)
			{
				eastl::vector<RE_GameObject*> all = go->GetActiveChildsWithDrawComponents();

				for (auto draw_go : all)
				{
					if (draw_go->IsStatic())
						static_tree.PopNode(goManager.WhatID(draw_go));
					else
						dynamic_tree.PopNode(goManager.WhatID(draw_go));
				}
			}
			break;
		}
		case GO_CHANGED_TO_STATIC:
		{
			if (belongs_to_scene && go->IsActive())
			{
				int index = goManager.WhatID(go);
				dynamic_tree.PopNode(index);
				static_tree.PushNode(index, go->GetGlobalBoundingBox());
			}
			break;
		}
		case GO_CHANGED_TO_NON_STATIC:
		{
			if (belongs_to_scene && go->IsActive())
			{
				int index = goManager.WhatID(go);
				static_tree.PopNode(index);
				dynamic_tree.PushNode(index, go->GetGlobalBoundingBox());
			}
			break;
		}
		case GO_HAS_NEW_CHILD:
		{
			RE_GameObject* to_add = e.data2.AsGO();

			if (belongs_to_scene && go->IsActive() && to_add->IsActive())
			{
				eastl::vector<RE_GameObject*> all = to_add->GetActiveChildsWithDrawComponents();

				for (auto draw_go : all)
				{
					draw_go->ResetGlobalBoundingBox();

					if (draw_go->IsStatic())
						static_tree.PushNode(goManager.WhatID(draw_go), draw_go->GetGlobalBoundingBox());
					else
						dynamic_tree.PushNode(goManager.WhatID(draw_go), draw_go->GetGlobalBoundingBox());
				}
			}

			break;
		}
		case GO_REMOVE_CHILD:
		{
			RE_GameObject* to_remove = e.data2.AsGO();

			if (belongs_to_scene && to_remove->IsActive())
			{
				eastl::vector<RE_GameObject*> all = to_remove->GetActiveChildsWithDrawComponents();

				for (auto draw_go : all)
				{
					if (draw_go->IsStatic())
						static_tree.PopNode(goManager.WhatID(draw_go));
					else
						dynamic_tree.PopNode(goManager.WhatID(draw_go));
				}
			}

			// TODO: Delete to_remove & childs from GO Pool

			break;
		}
		case TRANSFORM_MODIFIED:
		{
			if (go->IsActive())
				for (auto child : go->GetChilds())
					child->TransformModified();

			if (belongs_to_scene && go->HasDrawComponents())
			{
				int index = goManager.WhatID(go);
				if (go->IsStatic())
				{
					static_tree.PopNode(index);
					static_tree.PushNode(index, go->GetGlobalBoundingBox());
				}
				else
				{
					dynamic_tree.PopNode(index);
					dynamic_tree.PushNode(index, go->GetGlobalBoundingBox());
				}
			}
			haschanges = true;
			break;
		}
		case PLANE_CHANGE_TO_MESH:
		{
			RE_CompPlane* plane = (RE_CompPlane*)go->GetComponent(C_PLANE);
			const char* planeMD5 = plane->TransformAsMeshResource();
			go->RemoveComponent(plane);
			RE_CompMesh* newMesh = new RE_CompMesh(go, planeMD5);
			newMesh->UseResources();
			go->AddCompMesh(newMesh);
			go->ResetBoundingBoxes();
			go->TransformModified();
			haschanges = true;
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
	Event::Push(GO_HAS_NEW_CHILD, this, root, plane_go);
	plane_go->AddComponent(App->primitives->CreatePlane(plane_go));
	plane_go->ResetBoundingBoxes();
	plane_go->TransformModified(false);
	goManager.Push(plane_go);
}

void ModuleScene::CreateCube()
{
	RE_GameObject* cube_go = AddGO("Cube", root);
	Event::Push(GO_HAS_NEW_CHILD, this, root, cube_go);
	cube_go->AddComponent(App->primitives->CreateCube(cube_go));
	cube_go->ResetBoundingBoxes();
	cube_go->TransformModified(false);
	goManager.Push(cube_go);
}

void ModuleScene::CreateSphere()
{
	RE_GameObject* sphere_go = AddGO("Sphere", root);
	Event::Push(GO_HAS_NEW_CHILD, this, root, sphere_go);
	sphere_go->AddComponent(App->primitives->CreateSphere(sphere_go));
	sphere_go->ResetBoundingBoxes();
	sphere_go->TransformModified(false);
	goManager.Push(sphere_go);
}

void ModuleScene::CreateCamera()
{
	RE_GameObject* cam_go = AddGO("Camera", root);
	Event::Push(GO_HAS_NEW_CHILD, this, root, cam_go);
	cam_go->AddCompCamera();
	goManager.Push(cam_go);
}

void ModuleScene::DrawEditor()
{
	if (ImGui::CollapsingHeader(GetName()))
	{
		int total_count = goManager.GetCount();
		int static_count = static_tree.GetCount();
		int dynamic_count = dynamic_tree.GetCount();

		static_count = static_count <= 1 ? static_count : (static_count - 1) / 2;
		dynamic_count = dynamic_count <= 1 ? dynamic_count : (dynamic_count - 1) / 2;

		ImGui::Text("Total Scene GOs: %i", total_count);
		ImGui::Text("Total Active: %i", static_count + dynamic_count);
		ImGui::Text(" - Static: %i", static_count);
		ImGui::Text(" - Non Static: %i", dynamic_count);
		ImGui::Text("Total Inacive: %i", total_count - (static_count + dynamic_count));
	}
}

void ModuleScene::DrawTrees() const
{
	static_tree.Draw();
	dynamic_tree.Draw();
}

RE_GameObject* ModuleScene::RayCastSelect(math::Ray & ray)
{
	RE_GameObject* ret = nullptr;

	eastl::stack<int> goIndex;
	static_tree.CollectIntersections(ray, goIndex);
	dynamic_tree.CollectIntersections(ray, goIndex);

	eastl::vector<RE_GameObject*> objects;
	while (!goIndex.empty())
	{
		int index = goIndex.top();
		goIndex.pop();
		objects.push_back(goManager.At(index));
	}

	if (!objects.empty())
	{
		// Check Ray-Triangle
		bool collision = false;
		float closest_distance = -1.f;
		float res_distance = 0.0f;

		for (auto ob : objects)
		{
			RE_CompMesh* mesh = ob->GetMesh();

			if (mesh != nullptr)
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

void ModuleScene::FustrumCulling(eastl::vector<const RE_GameObject*>& container, const math::Frustum & frustum)
{
	eastl::stack<int> goIndex;
	static_tree.CollectIntersections(frustum, goIndex);
	dynamic_tree.CollectIntersections(frustum, goIndex);

	while (!goIndex.empty())
	{
		int index = goIndex.top();
		goIndex.pop();
		container.push_back(goManager.At(index));
	}
}

void ModuleScene::SaveScene(const char* newName)
{
	RE_Scene* scene = (unsavedScene) ? unsavedScene : (RE_Scene*)App->resources->At(currentScene);

	scene->SetName((unsavedScene) ? (newName) ? newName : root->GetName() : scene->GetName());
	scene->Save(root);
	scene->SaveMeta();

	if (unsavedScene) {
		currentScene = App->resources->Reference(scene);
		App->thumbnail->Add(currentScene);
		unsavedScene = nullptr;
	}
	else
		App->thumbnail->Change(scene->GetMD5());

	haschanges = false;
}

const char* ModuleScene::GetCurrentScene() const
{
	return currentScene;
}

void ModuleScene::ClearScene()
{
	Event::PauseEvents();

	goManager.Clear();
	static_tree.Clear();
	dynamic_tree.Clear();

	if (root) {
		root->UnUseResources();
		DEL(root);
	}
	if (savedState) DEL(savedState);

	root = new RE_GameObject("root");
	App->editor->SetSelected(root);
	RE_GameObject* cam_go = AddGO("Camera", root);
	cam_go->AddCompCamera();
	App->cams->RecallCameras(root);

	Event::ResumeEvents();
}

void ModuleScene::NewEmptyScene(const char* name)
{
	Event::PauseEvents();

	App->editor->ClearCommands();

	if (unsavedScene)  //TODO Needs popUp for alert to save or not.
	{
		DEL(unsavedScene);
	}
	else if (currentScene) { //TODO popup save
		currentScene = nullptr;
	}
	
	unsavedScene = new RE_Scene();
	unsavedScene->SetName(name);
	unsavedScene->SetType(Resource_Type::R_SCENE);

	if (root) {
		root->UnUseResources();
		DEL(root);
	}
	if (savedState) DEL(savedState);
	goManager.Clear();

	root = new RE_GameObject("root");
	root->SetStatic(false);

	SetupScene();
	App->editor->SetSelected(root);

	Event::ResumeEvents();

	haschanges = false;
}

void ModuleScene::LoadScene(const char* sceneMD5, bool ignorehandle)
{
	Event::PauseEvents();

	App->editor->ClearCommands();

	if (unsavedScene) {
		DEL(unsavedScene);
	}

	if (root) {
		root->UnUseResources();
		DEL(root);
	}
	if (savedState) DEL(savedState);
	goManager.Clear();

	LOG("Loading scene from own format:");
	if(!ignorehandle) App->handlerrors->StartHandling();

	Timer timer;

	currentScene = sceneMD5;
	RE_Scene* scene = (RE_Scene*)App->resources->At(currentScene);
	RE_GameObject* loadedDO = scene->GetRoot();

	if (loadedDO)
		root = loadedDO;
	else 
		LOG_ERROR("Can´t Load Scene");

	root->UseResources();
	SetupScene();
	App->editor->SetSelected(root);

	LOG("Time loading scene: %u ms", timer.Read());

	if (!ignorehandle) {
		// Error Handling
		App->handlerrors->StopHandling();
		if (App->handlerrors->AnyErrorHandled()) {
			App->handlerrors->ActivatePopUp();
		}
	}

	Event::ResumeEvents();
}

void ModuleScene::SetupScene()
{
	Event::PauseEvents();

	// Render Camera Management
	App->cams->RecallCameras(root);
	if (!RE_CameraManager::HasMainCamera()) {
		Event::ResumeEvents();
		CreateCamera();
		Event::PauseEvents();
	}

	goManager.PushWithChilds(root);

	if (savedState) DEL(savedState);
	savedState = new RE_GameObject(*root);

	// Setup Tree AABBs
	root->TransformModified(false);
	root->Update();
	root->ResetBoundingBoxForAllChilds();
	ResetTrees();

	Event::ResumeEvents();
}

void ModuleScene::AddGameobject(RE_GameObject* toAdd)
{
	//TODO don't use SetupScene, only setup toAdd
	//goManager.PushWithChilds(toAdd);
	toAdd->UseResources();
	root->AddChild(toAdd);
	goManager.Clear();

	SetupScene();
	App->editor->SetSelected(toAdd);

	haschanges = true;
}

bool ModuleScene::HasChanges() const
{
	return haschanges;
}

bool ModuleScene::isNewScene() const
{
	return (unsavedScene);
}

void ModuleScene::GetActive(eastl::list<RE_GameObject*>& objects) const
{
	eastl::queue<RE_GameObject*> queue;

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

void ModuleScene::GetActiveStatic(eastl::list<RE_GameObject*>& objects) const
{
	eastl::queue<RE_GameObject*> queue;

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

void ModuleScene::GetActiveNonStatic(eastl::list<RE_GameObject*>& objects) const
{
	eastl::queue<RE_GameObject*> queue;

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

void ModuleScene::ResetTrees()
{
	static_tree.Clear();
	dynamic_tree.Clear();

	eastl::vector<int> goIndex = goManager.GetAllKeys();
	for (int i = 0; i < goIndex.size(); i++)
	{
		RE_GameObject* go = goManager.At(goIndex[i]);

		if (go->IsActive() && go->HasDrawComponents())
		{
			if (go->IsStatic())
				static_tree.PushNode(goIndex[i], go->GetGlobalBoundingBox());
			else
				dynamic_tree.PushNode(goIndex[i], go->GetGlobalBoundingBox());
		}
	}
}

void GameObjectManager::Clear()
{
	goToID.clear();
	poolmapped_.clear();
	lastAvaibleIndex = 0;
}

eastl::map< RE_GameObject*, int> GameObjectManager::PushWithChilds(RE_GameObject* val, bool root)
{
	eastl::map< RE_GameObject*, int> ret;
	eastl::vector<RE_GameObject*> gos =  val->GetAllGO();
	 if (!root) gos.erase(gos.begin());
	 for (auto go : gos) ret.insert(eastl::pair<RE_GameObject*,int>(go, Push(go)));
	 return ret;
}

int GameObjectManager::Push(RE_GameObject* val)
{
	int ret = lastAvaibleIndex;
	goToID.insert(eastl::pair<RE_GameObject*, int>(val, ret));
	PoolMapped::Push(val, ret);
	return ret;
}

eastl::vector<RE_GameObject*> GameObjectManager::PopWithChilds(int id, bool root)
{
	eastl::vector<RE_GameObject*> gos = At(id)->GetAllGO();
	if (!root) gos.erase(gos.begin());
	for (auto go : gos) Pop(goToID.at(go));
	return gos;
}

RE_GameObject* GameObjectManager::Pop(int id)
{
	RE_GameObject* ret = PoolMapped::Pop(id);
	goToID.erase(ret);
	return ret;
}

int GameObjectManager::WhatID(RE_GameObject* go) const
{
	return goToID.at(go);
}
