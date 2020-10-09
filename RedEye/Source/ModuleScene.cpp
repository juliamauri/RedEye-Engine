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
#include "RE_GOManager.h"

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
	if (unsavedScene) DEL(unsavedScene);
	return true;
}

void ModuleScene::OnPlay()
{
	Event::PauseEvents();
	savedState.ClearPool();
	savedState.InsertPool(&scenePool);
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

	scenePool.ClearPool();
	root = scenePool.InsertPool(&savedState);
	App->editor->SetSelected(nullptr);
	savedState.ClearPool();
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
						static_tree.PushNode(draw_go->GetPoolID(), draw_go->GetGlobalBoundingBox());
					else
						dynamic_tree.PushNode(draw_go->GetPoolID(), draw_go->GetGlobalBoundingBox());
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
						static_tree.PopNode(draw_go->GetPoolID());
					else
						dynamic_tree.PopNode(draw_go->GetPoolID());
				}
			}
			break;
		}
		case GO_CHANGED_TO_STATIC:
		{
			if (belongs_to_scene && go->IsActive())
			{
				int index = go->GetPoolID();
				dynamic_tree.PopNode(index);
				static_tree.PushNode(index, go->GetGlobalBoundingBox());
			}
			break;
		}
		case GO_CHANGED_TO_NON_STATIC:
		{
			if (belongs_to_scene && go->IsActive())
			{
				int index = go->GetPoolID();
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
						static_tree.PushNode(draw_go->GetPoolID(), draw_go->GetGlobalBoundingBox());
					else
						dynamic_tree.PushNode(draw_go->GetPoolID(), draw_go->GetGlobalBoundingBox());
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
						static_tree.PopNode(draw_go->GetPoolID());
					else
						dynamic_tree.PopNode(draw_go->GetPoolID());
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
				int index = go->GetPoolID();
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
			RE_CompMesh* newMesh = go->AddCompMesh();
			newMesh->SetUp(go, planeMD5);
			newMesh->UseResources();
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

void ModuleScene::CreateCube(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* cube_go = scenePool.AddGO("Cube", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, cube_go);

	cube_go->AddComponent(C_CUBE);
	cube_go->ResetBoundingBoxes();
	cube_go->TransformModified(false);
}

void ModuleScene::CreateDodecahedron(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* dode_go = scenePool.AddGO("Dodecahedron", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, dode_go);

	dode_go->AddComponent(C_DODECAHEDRON);
	dode_go->ResetBoundingBoxes();
	dode_go->TransformModified(false);
}

void ModuleScene::CreateTetrahedron(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* tetra_go = scenePool.AddGO("Tetrahedron", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, tetra_go);

	tetra_go->AddComponent(C_TETRAHEDRON);
	tetra_go->ResetBoundingBoxes();
	tetra_go->TransformModified(false);
}

void ModuleScene::CreateOctohedron(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* octo_go = scenePool.AddGO("Octohedron", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, octo_go);

	octo_go->AddComponent(C_OCTOHEDRON);
	octo_go->ResetBoundingBoxes();
	octo_go->TransformModified(false);
}

void ModuleScene::CreateIcosahedron(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* icosa_go = scenePool.AddGO("Icosahedron", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, icosa_go);

	icosa_go->AddComponent(C_ICOSAHEDRON);
	icosa_go->ResetBoundingBoxes();
	icosa_go->TransformModified(false);
}

void ModuleScene::CreatePlane(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* plane_go = scenePool.AddGO("Plane", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, plane_go);

	plane_go->AddComponent(C_PLANE);
	plane_go->ResetBoundingBoxes();
	plane_go->TransformModified(false);
}

void ModuleScene::CreateSphere(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* sphere_go = scenePool.AddGO("Sphere", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, sphere_go);

	sphere_go->AddComponent(C_SPHERE);
	sphere_go->ResetBoundingBoxes();
	sphere_go->TransformModified(false);
}

void ModuleScene::CreateCylinder(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* cylinder_go = scenePool.AddGO("Cylinder", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, cylinder_go);

	cylinder_go->AddComponent(C_CYLINDER);
	cylinder_go->ResetBoundingBoxes();
	cylinder_go->TransformModified(false);
}

void ModuleScene::CreateHemiSphere(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* hemisphere_go = scenePool.AddGO("HemiSphere", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, hemisphere_go);

	hemisphere_go->AddComponent(C_HEMISHPERE);
	hemisphere_go->ResetBoundingBoxes();
	hemisphere_go->TransformModified(false);
}

void ModuleScene::CreateTorus(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* torus_go = scenePool.AddGO("Torus", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, torus_go);

	torus_go->AddComponent(C_TORUS);
	torus_go->ResetBoundingBoxes();
	torus_go->TransformModified(false);
}

void ModuleScene::CreateTrefoilKnot(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* trefoilknot_go = scenePool.AddGO("Trefoil Knot", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, trefoilknot_go);

	trefoilknot_go->AddComponent(C_TREFOILKNOT);
	trefoilknot_go->ResetBoundingBoxes();
	trefoilknot_go->TransformModified(false);
}

void ModuleScene::CreateRock(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* rock_go = scenePool.AddGO("Rock", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, rock_go);

	rock_go->AddComponent(C_ROCK);
	rock_go->ResetBoundingBoxes();
	rock_go->TransformModified(false);
}

void ModuleScene::CreateCamera(RE_GameObject* parent)
{
	parent = (parent) ? parent : root;
	RE_GameObject* cam_go = scenePool.AddGO("Camera", parent);
	Event::Push(GO_HAS_NEW_CHILD, this, parent, cam_go);
	cam_go->AddCompCamera();
}

void ModuleScene::DrawEditor()
{
	if (ImGui::CollapsingHeader(GetName()))
	{
		int total_count = scenePool.TotalGameObjects();
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
		objects.push_back(scenePool.GetGO(index));
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
		container.push_back(goManager.At(goIndex.top()));
		goIndex.pop();
	}
}

void ModuleScene::SaveScene(const char* newName)
{
	RE_Scene* scene = (unsavedScene) ? unsavedScene : (RE_Scene*)App->resources->At(currentScene);

	Event::PauseEvents();

	scene->SetName((unsavedScene) ? (newName) ? newName : root->GetName() : scene->GetName());
	scene->Save(&scenePool);
	scene->SaveMeta();

	if (unsavedScene) {
		currentScene = App->resources->Reference(scene);
		App->thumbnail->Add(currentScene);
		unsavedScene = nullptr;
	}
	else
		App->thumbnail->Change(scene->GetMD5());

	haschanges = false;
	Event::ResumeEvents();
}

const char* ModuleScene::GetCurrentScene() const
{
	return currentScene;
}

void ModuleScene::ClearScene()
{
	Event::PauseEvents();

	if (root) scenePool.UnUseResources();
	savedState.ClearPool();

	static_tree.Clear();
	dynamic_tree.Clear();
	scenePool.ClearPool();

	root = scenePool.AddGO("root", nullptr);
	root->SetStatic(false);
	App->editor->SetSelected(root);

	RE_GameObject* cam_go = scenePool.AddGO("Camera", root);
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

	if (root) scenePool.UnUseResources();
	savedState.ClearPool();
	scenePool.ClearPool();

	root = scenePool.AddGO("root", nullptr);
	root->SetStatic(false);

	SetupScene();
	App->editor->SetSelected(nullptr);

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

	if (root) scenePool.UnUseResources();
	savedState.ClearPool();
	scenePool.ClearPool();

	LOG("Loading scene from own format:");
	if(!ignorehandle) App->handlerrors->StartHandling();

	Timer timer;

	currentScene = sceneMD5;
	RE_Scene* scene = (RE_Scene*)App->resources->At(currentScene);
	App->resources->Use(sceneMD5);
	RE_GOManager* loadedDO = scene->GetPool();

	if (loadedDO)
		root = scenePool.InsertPool(loadedDO);
	else
		LOG_ERROR("Can�t Load Scene");
	App->resources->UnUse(sceneMD5);

	scenePool.UseResources();
	SetupScene();
	App->editor->SetSelected(nullptr);

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

	savedState.ClearPool();
	savedState.InsertPool(&scenePool);

	// Setup Tree AABBs
	root->TransformModified(false);
	root->Update();
	root->ResetBoundingBoxForAllChilds();
	ResetTrees();

	Event::ResumeEvents();
}

void ModuleScene::AddGOPool(RE_GOManager* toAdd)
{
	//TODO don't use SetupScene, only setup toAdd
	//App->goManager->sceneGOs.PushWithChilds(toAdd);

	RE_GameObject* justAdded = scenePool.InsertPool(toAdd);

	scenePool.UseResources();

	SetupScene();
	App->editor->SetSelected(justAdded);

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

RE_GOManager* ModuleScene::GetScenePool()
{
	return &scenePool;
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

	eastl::vector<int> goIndex = scenePool.GetAllGOs();
	for (int i = 0; i < goIndex.size(); i++)
	{
		RE_GameObject* go = scenePool.GetGO(goIndex[i]);

		if (go->IsActive() && go->HasDrawComponents())
		{
			if (go->IsStatic())
				static_tree.PushNode(goIndex[i], go->GetGlobalBoundingBox());
			else
				dynamic_tree.PushNode(goIndex[i], go->GetGlobalBoundingBox());
		}
	}
}
