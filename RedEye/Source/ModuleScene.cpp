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
#include "RE_TimeManager.h"

#include "md5.h"
#include <EASTL/string.h>
#include <EASTL/queue.h>
#include <EASTL/vector.h>
#include "SDL2\include\SDL.h"

#define DEFAULTMODEL "Assets/Meshes/BakerHouse/BakerHouse.fbx"

RE_GOManager ModuleScene::scenePool;

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled) {}
ModuleScene::~ModuleScene() {}

bool ModuleScene::Start()
{
	eastl::vector<ResourceContainer*> scenes = App::resources->GetResourcesByType(R_SCENE);
	if (!scenes.empty()) LoadScene(scenes[0]->GetMD5());
	else NewEmptyScene();
	return true;
}

update_status ModuleScene::Update()
{
	OPTICK_CATEGORY("Update Scene", Optick::Category::GameLogic);
	scenePool.Update();
	return UPDATE_CONTINUE;
}

update_status ModuleScene::PostUpdate()
{
	bool someDelete = !to_delete.empty();
	while (!to_delete.empty())
	{
		scenePool.DestroyGO(to_delete.top());
		to_delete.pop();
	}
	if (someDelete) SetupScene();

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	if (unsavedScene) DEL(unsavedScene);
	return true;
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

void ModuleScene::DebugDraw() const
{
	static_tree.Draw();
	dynamic_tree.Draw();
}

void ModuleScene::OnPlay()
{
	Event::PauseEvents();
	savedState.ClearPool();
	savedState.InsertPool(&scenePool);
	Event::ResumeEvents();

	scenePool.GetRootPtr()->OnPlay();
}

void ModuleScene::OnPause()
{
	scenePool.GetRootPtr()->OnPause();
}

void ModuleScene::OnStop()
{
	scenePool.GetRootPtr()->OnStop();

	Event::PauseEvents();
	scenePool.ClearPool();
	scenePool.InsertPool(&savedState);
	savedState.ClearPool();
	App::editor->SetSelected(0);
	SetupScene();
	Event::ResumeEvents();
}

void ModuleScene::RecieveEvent(const Event& e)
{
	//UID go_uid = e.data1.AsUInt64();

	switch (e.type)
	{
	case GO_CHANGED_TO_ACTIVE:
	{
		RE_GameObject* go = scenePool.GetGOPtr(e.data1.AsUInt64());
		eastl::vector<RE_GameObject*> all = go->GetActiveDrawableGOandChildsPtr();

		for (auto draw_go : all)
		{
			draw_go->ResetGlobalBoundingBox();
			(draw_go->IsStatic() ? static_tree : dynamic_tree).PushNode(draw_go->GetUID(), draw_go->GetGlobalBoundingBox());
		}

		haschanges = true;
		break;
	}
	case GO_CHANGED_TO_INACTIVE:
	{
		eastl::vector<RE_GameObject*> all = scenePool.GetGOPtr(e.data1.AsUInt64())->GetActiveDrawableGOandChildsPtr();

		for (auto draw_go : all)
			(draw_go->IsStatic() ? static_tree : dynamic_tree).PopNode(draw_go->GetUID());

		haschanges = true;
		break;
	}
	case GO_CHANGED_TO_STATIC:
	{
		UID go_uid = e.data1.AsUInt64();
		RE_GameObject* go = scenePool.GetGOPtr(go_uid);
		if (go->IsActive())
		{
			dynamic_tree.PopNode(go_uid);
			static_tree.PushNode(go_uid, go->GetGlobalBoundingBox());
		}

		haschanges = true;
		break;
	}
	case GO_CHANGED_TO_NON_STATIC:
	{
		UID go_uid = e.data1.AsUInt64();
		RE_GameObject* go = scenePool.GetGOPtr(go_uid);
		if (go->IsActive())
		{
			static_tree.PopNode(go_uid);
			dynamic_tree.PushNode(go_uid, go->GetGlobalBoundingBox());
		}

		haschanges = true;
		break;
	}
	case GO_HAS_NEW_CHILD:
	{
		RE_GameObject* go = scenePool.GetGOPtr(e.data1.AsUInt64());
		UID child_uid = e.data2.AsUInt64();
		RE_GameObject* child = scenePool.GetGOPtr(child_uid);
		if (go->IsActive() && child->IsActive())
			(child->IsStatic() ? static_tree : dynamic_tree).PushNode(child_uid, child->GetGlobalBoundingBox());

		haschanges = true;
		break;
	}
	case DESTROY_GO:
	{
		UID go_uid = e.data1.AsUInt64();
		RE_GameObject* go = scenePool.GetGOPtr(go_uid);

		if (go->IsActive())
		{
			eastl::vector<const RE_GameObject*> all = go->GetActiveDrawableGOandChildsCPtr();
			for (auto draw_go : all) (draw_go->IsStatic() ? static_tree : dynamic_tree).PopNode(draw_go->GetUID());
		}

		if (App::editor->GetSelected() == go_uid) App::editor->SetSelected(0);

		to_delete.push(go_uid);
		haschanges = true;
		break;
	}
	case TRANSFORM_MODIFIED:
	{
		RE_GameObject* go = scenePool.GetGOPtr(e.data1.AsUInt64());
		if (go->IsActive())
			for (auto child : go->GetChildsPtr())
				child->TransformModified();

		if (go->HasRenderGeo())
		{
			UID index = go->GetUID();
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
		RE_GameObject* go = scenePool.GetGOPtr(e.data1.AsUInt64());
		RE_CompPlane* plane = dynamic_cast<RE_CompPlane*>(go->GetCompPtr(C_PLANE));
		const char* planeMD5 = plane->TransformAsMeshResource();
		go->DestroyComponent(plane->GetPoolID(), C_PLANE);
		RE_CompMesh* newMesh = dynamic_cast<RE_CompMesh*>(go->AddNewComponent(C_MESH));
		newMesh->SetMesh(planeMD5);
		newMesh->UseResources();
		haschanges = true;
		break;
	}
	}
}

RE_GOManager* ModuleScene::GetScenePool() { return &scenePool; }
RE_GameObject* ModuleScene::GetGOPtr(UID id) { return scenePool.GetGOPtr(id); }
const RE_GameObject* ModuleScene::GetGOCPtr(UID id) { return scenePool.GetGOCPtr(id); }
UID ModuleScene::GetRootUID() { return scenePool.GetRootUID(); }
RE_GameObject * ModuleScene::GetRootPtr() { return scenePool.GetRootPtr(); }
const RE_GameObject * ModuleScene::GetRootCPtr() { return scenePool.GetRootCPtr(); }

void ModuleScene::CreatePrimitive(ComponentType type, const UID parent)
{
	scenePool.AddGO("Primitive", Validate(parent), true)->AddNewComponent(type);
}

void ModuleScene::CreateCamera(const UID parent)
{
	scenePool.AddGO("Camera", Validate(parent), true)->AddNewComponent(C_CAMERA);
}

void ModuleScene::CreateLight(const UID parent)
{
	RE_GameObject* light_go = scenePool.AddGO("Light", Validate(parent), true);

	dynamic_cast<RE_CompPrimitive*>(light_go->AddNewComponent(C_SPHERE))->SetColor(
		dynamic_cast<RE_CompLight*>(light_go->AddNewComponent(C_LIGHT))->diffuse);
}

void ModuleScene::CreateMaxLights(const UID parent)
{
	RE_GameObject* container_go = scenePool.AddGO("Bunch of Lights", (parent) ? parent : GetRootUID());

	eastl::string name = "light ";
	for (int x = 0; x < 8; ++x)
	{
		for (int y = 0; y < 8; ++y)
		{
			RE_GameObject* light_go = scenePool.AddGO((name + eastl::to_string(x) + "x" + eastl::to_string(y)).c_str(), container_go->GetUID());
			light_go->GetTransformPtr()->SetPosition(math::vec((static_cast<float>(x) * 12.5f) - 50.f, 0.f, (static_cast<float>(y) * 12.5f) - 50.f));

			static math::vec colors[5] = { math::vec(1.f,0.f,0.f), math::vec(0.f,1.f,0.f), math::vec(0.f,0.f,1.f), math::vec(1.f,1.f,0.f), math::vec(0.f,1.f,1.f) };
			dynamic_cast<RE_CompPrimitive*>(light_go->AddNewComponent(C_SPHERE))->SetColor(
				dynamic_cast<RE_CompLight*>(light_go->AddNewComponent(C_LIGHT))->diffuse = colors[RE_Math::RandomInt() % 5]);
		}
	}
}

void ModuleScene::AddGOPool(RE_GOManager* toAdd)
{
	//TODO Julius don't use SetupScene, only setup toAdd
	//App::goManager->sceneGOs.PushWithChilds(toAdd);

	RE_GameObject* justAdded = scenePool.InsertPool(toAdd);
	scenePool.UseResources();
	SetupScene();
	App::editor->SetSelected(justAdded->GetUID());
	haschanges = true;
}

UID ModuleScene::RayCastSelect(math::Ray & global_ray)
{
	UID ret = 0;

	eastl::stack<UID> aabb_collisions;
	static_tree.CollectIntersections(global_ray, aabb_collisions);
	dynamic_tree.CollectIntersections(global_ray, aabb_collisions);

	eastl::vector<eastl::pair<UID,const RE_GameObject*>> gos;
	while (!aabb_collisions.empty())
	{
		UID id = aabb_collisions.top();
		aabb_collisions.pop();
		gos.push_back({ id, scenePool.GetGOCPtr(id) });
	}

	if (!gos.empty())
	{
		// Check Ray-Triangle
		bool collision = false;
		float res_distance, closest_distance = -1.f;
		for (auto go : gos)
		{
			res_distance = 0.f;
			if (go.second->CheckRayCollision(global_ray, res_distance) && (!collision || res_distance < closest_distance))
			{
				ret = go.first;
				closest_distance = res_distance;
				collision = true;
			}
		}
	}

	return ret;
}

void ModuleScene::FustrumCulling(eastl::vector<const RE_GameObject*>& container, const math::Frustum & frustum)
{
	eastl::stack<UID> goIndex;
	static_tree.CollectIntersections(frustum, goIndex);
	dynamic_tree.CollectIntersections(frustum, goIndex);

	while (!goIndex.empty())
	{
		container.push_back(scenePool.GetGOCPtr(goIndex.top()));
		goIndex.pop();
	}
}

void ModuleScene::SaveScene(const char* newName)
{
	RE_Scene* scene = (unsavedScene) ? unsavedScene : dynamic_cast<RE_Scene*>(App::resources->At(currentScene));

	Event::PauseEvents();

	scene->SetName((unsavedScene) ? (newName) ? newName : GetRootCPtr()->name.c_str() : scene->GetName());
	scene->Save(&scenePool);
	scene->SaveMeta();

	if (unsavedScene)
	{
		currentScene = App::resources->Reference(scene);
		App::thumbnail->Add(currentScene);
		unsavedScene = nullptr;
	}
	else
		App::thumbnail->Change(scene->GetMD5());

	haschanges = false;
	Event::ResumeEvents();
}

const char* ModuleScene::GetCurrentScene() const { return currentScene; }

void ModuleScene::ClearScene()
{
	Event::PauseEvents();

	if (GetRootUID()) scenePool.UnUseResources();
	savedState.ClearPool();

	static_tree.Clear();
	dynamic_tree.Clear();
	scenePool.ClearPool();

	RE_GameObject* root = scenePool.AddGO("root", 0);
	root->SetStatic(false);

	UID root_uid = scenePool.GetRootUID();
	scenePool.AddGO("Camera", root_uid)->AddNewComponent(C_CAMERA);
	App::editor->SetSelected(root_uid);

	RE_CameraManager::RecallSceneCameras();

	Event::ResumeEvents();
}

void ModuleScene::NewEmptyScene(const char* name)
{
	Event::PauseEvents();
	App::editor->ClearCommands();

	if (unsavedScene) { DEL(unsavedScene); }
	else currentScene = nullptr; /* TODO Julius popup save */

	unsavedScene = new RE_Scene();
	unsavedScene->SetName(name);
	unsavedScene->SetType(R_SCENE);

	scenePool.UnUseResources();
	savedState.ClearPool();
	scenePool.ClearPool();

	scenePool.AddGO("root", 0)->SetStatic(false);

	SetupScene();
	App::editor->SetSelected(0);
	Event::ResumeEvents();

	haschanges = false;
}

void ModuleScene::LoadScene(const char* sceneMD5, bool ignorehandle)
{
	Event::PauseEvents();
	App::editor->ClearCommands();
	if (unsavedScene) DEL(unsavedScene);
	
	scenePool.UnUseResources();
	savedState.ClearPool();
	scenePool.ClearPool();

	RE_LOG("Loading scene from own format:");
	if(!ignorehandle) App::handlerrors.StartHandling();
	Timer timer;
	currentScene = sceneMD5;
	RE_Scene* scene = dynamic_cast<RE_Scene*>(App::resources->At(currentScene));
	App::resources->Use(sceneMD5);

	RE_GOManager* loadedDO = scene->GetPool();
	if (loadedDO)scenePool.InsertPool(loadedDO);
	else RE_LOG_ERROR("Can't Load Scene");

	App::resources->UnUse(sceneMD5);
	scenePool.UseResources();
	SetupScene();
	App::editor->SetSelected(0);

	RE_LOG("Time loading scene: %u ms", timer.Read());
	if (!ignorehandle) App::handlerrors.StopAndPresent();
	Event::ResumeEvents();
}

void ModuleScene::SetupScene()
{
	Event::PauseEvents();

	// Render Camera Management
	RE_CameraManager::RecallSceneCameras();
	if (!RE_CameraManager::HasMainCamera())
	{
		Event::ResumeEvents();
		CreateCamera(scenePool.GetRootUID());
		Event::PauseEvents();
	}

	savedState.ClearPool();
	savedState.InsertPool(&scenePool);

	// Setup Tree AABBs¡
	static_tree.Clear();
	dynamic_tree.Clear();
	GetRootPtr()->ResetBoundingBoxForAllChilds();

	eastl::vector<eastl::pair<UID, RE_GameObject*>> gos = scenePool.GetAllGOData();
	for (unsigned int i = 0; i < gos.size(); i++)
	{
		if (gos[i].second->HasActiveRenderGeo())
			(gos[i].second->IsStatic() ? static_tree : dynamic_tree).PushNode(gos[i].first, gos[i].second->GetGlobalBoundingBox());
	}

	Event::ResumeEvents();
}

inline UID ModuleScene::Validate(const UID id)
{
	return id ? id : GetRootUID();
}

bool ModuleScene::HasChanges() const { return haschanges; }
bool ModuleScene::isNewScene() const { return (unsavedScene); }

/*void ModuleScene::GetActive(eastl::list<RE_GameObject*>& objects) const
{
	eastl::queue<RE_GameObject*> queue;

	for (auto child : root->GetChildsPtr())
		if (child->IsActive())
			queue.push(child);

	while (!queue.empty())
	{
		RE_GameObject* obj = queue.front();
		objects.push_back(obj);

		for (auto child : obj->GetChildsPtr())
			if (child->IsActive())
				queue.push(child);

		queue.pop();
	}
}

void ModuleScene::GetActiveStatic(eastl::list<RE_GameObject*>& objects) const
{
	eastl::queue<RE_GameObject*> queue;

	for (auto child : root->GetChildsPtr())
		if (child->IsActiveStatic())
			queue.push(child);

	while (!queue.empty())
	{
		RE_GameObject* obj = queue.front();
		objects.push_back(obj);

		for (auto child : obj->GetChildsPtr())
			if (child->IsActiveStatic())
				queue.push(child);

		queue.pop();
	}
}

void ModuleScene::GetActiveNonStatic(eastl::list<RE_GameObject*>& objects) const
{
	eastl::queue<RE_GameObject*> queue;

	for (auto child : root->GetChildsPtr())
		if (child->IsActiveNonStatic())
			queue.push(child);

	while (!queue.empty())
	{
		RE_GameObject* obj = queue.front();
		objects.push_back(obj);

		for (auto child : obj->GetChildsPtr())
			if (child->IsActiveNonStatic())
				queue.push(child);

		queue.pop();
	}
}*/

