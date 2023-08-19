#include "Event.h"
#include <EASTL/stack.h>

#include "ModuleScene.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_Timer.h"
#include "RE_Math.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_CameraManager.h"
#include "RE_PrimitiveManager.h"
#include "RE_ShaderImporter.h"
#include "RE_ModelImporter.h"
#include "RE_TextureImporter.h"
#include "RE_ECS_Pool.h"
#include "RE_GameObject.h"
#include "RE_CompMesh.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CompPrimitive.h"
#include "RE_CompParticleEmitter.h"
#include "RE_ParticleEmitter.h"
#include "RE_Material.h"
#include "RE_Prefab.h"
#include "RE_Scene.h"
#include "RE_Model.h"

#include <SDL2/SDL.h>
#include <ImGui/imgui_internal.h>
#include <EASTL/string.h>
#include <EASTL/queue.h>
#include <EASTL/vector.h>

ModuleScene::ModuleScene() :
	cams(new RE_CameraManager()),
	primitives(new RE_PrimitiveManager()) {}

ModuleScene::~ModuleScene() { DEL(cams); DEL(primitives); }

bool ModuleScene::Init()
{
	RE_PROFILE(PROF_Init, PROF_ModuleScene);
	RE_LOG("Initializing Module Scene");
	cams->Init();
	primitives->Init();
	return true;
}

bool ModuleScene::Start()
{
	RE_PROFILE(PROF_Start, PROF_ModuleScene);
	RE_LOG("Starting Module Scene");

	eastl::vector<ResourceContainer*> scenes = RE_RES->GetResourcesByType(R_SCENE);
	if (!scenes.empty()) LoadScene(scenes[0]->GetMD5());
	else NewEmptyScene();

	return true;
}

void ModuleScene::Update()
{
	RE_PROFILE(PROF_Update, PROF_ModuleScene);
	scenePool.Update();
}

void ModuleScene::PostUpdate()
{
	RE_PROFILE(PROF_PostUpdate, PROF_ModuleScene);
	bool someDelete = !to_delete.empty();
	while (!to_delete.empty())
	{
		scenePool.DestroyGO(to_delete.top());
		to_delete.pop();
	}
}

void ModuleScene::CleanUp()
{
	RE_PROFILE(PROF_CleanUp, PROF_ModuleScene);
	cams->Clear();
	primitives->Clear();
	if (unsavedScene) DEL(unsavedScene);
}

void ModuleScene::DrawEditor()
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

	if (ImGui::TreeNodeEx("Transforms", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		eastl::vector<GO_UID> allTransforms = GetScenePool()->GetAllGOUIDs();

		int transformCount = allTransforms.size();
		ImGui::Text("Total %i transforms.", transformCount);
		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_::ImGuiSeparatorFlags_Vertical);
		static int range = 0, totalShowing = 8;
		ImGui::SameLine();

		ImGui::Text("Actual Range: %i - %i", range, (range + totalShowing < transformCount) ? range + totalShowing : transformCount);

		ImGui::PushItemWidth(50.f);
		ImGui::DragInt("Position", &range, 1.f, 0, transformCount - totalShowing);
		ImGui::SameLine();
		ImGui::DragInt("List Size", &totalShowing, 1.f, 0);

		if (ImGui::BeginTable("transformTable", 1, ImGuiTableFlags_::ImGuiTableFlags_BordersH | ImGuiTableFlags_Hideable)) {
			ImGui::TableNextColumn();
			for (int i = range; i < totalShowing + range && i < transformCount; i++) {
				RE_CompTransform* transform = GetGOPtr(allTransforms[i])->GetTransformPtr();

				ImGui::Text(("Name: " + transform->GetGOPtr()->name).c_str());

				if (ImGui::BeginTable("transformsTableNested", 2, ImGuiTableFlags_Hideable))
				{
					ImGui::TableSetupColumn("Local Transform:");
					ImGui::TableSetupColumn("Global Transform:");
					ImGui::TableHeadersRow();
					ImGui::TableNextRow(ImGuiTableRowFlags_None);

					ImGui::TableNextColumn();

					static math::vec pos, rotE, scl;
					static math::float3x3 rot;

					static math::float4x4 localM;
					localM = transform->GetLocalMatrix().Transposed();
					localM.Decompose(pos, rot, scl);
					rotE = rot.ToEulerXYZ();
					ImGui::Text("Position:");
					ImGui::Text("X %.3f | Y %.3f | Z %.3f", pos.x, pos.y, pos.z);
					ImGui::Text("Rotation:");
					ImGui::Text("X %.3f | Y %.3f | Z %.3f", math::RadToDeg(rotE.x), math::RadToDeg(rotE.y), math::RadToDeg(rotE.z));
					ImGui::Text("Scale:");
					ImGui::Text("X %.3f | Y %.3f | Z %.3f", scl.x, scl.y, scl.z);

					ImGui::TableNextColumn();

					static math::float4x4 globalM;
					globalM = transform->GetGlobalMatrix().Transposed();
					globalM.Decompose(pos, rot, scl);
					rotE = rot.ToEulerXYZ();
					ImGui::Text("Position:");
					ImGui::Text("X %.3f | Y %.3f | Z %.3f", pos.x, pos.y, pos.z);
					ImGui::Text("Rotation:");
					ImGui::Text("X %.3f | Y %.3f | Z %.3f", math::RadToDeg(rotE.x), math::RadToDeg(rotE.y), math::RadToDeg(rotE.z));
					ImGui::Text("Scale:");
					ImGui::Text("X %.3f | Y %.3f | Z %.3f", scl.x, scl.y, scl.z);

					ImGui::EndTable();
				}
				ImGui::TableNextColumn();
			}
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}
}

void ModuleScene::DrawSpacePartitioning() const
{
	static_tree.Draw();
	dynamic_tree.Draw();
}

void ModuleScene::OnPlay()
{
	RE_INPUT->PauseEvents();
	savedState.ClearPool();
	savedState.InsertPool(&scenePool);
	RE_INPUT->ResumeEvents();

	scenePool.GetRootPtr()->OnPlay();

	is_playing = true;
}

void ModuleScene::OnPause()
{
	scenePool.GetRootPtr()->OnPause();

	is_playing = true;
}

void ModuleScene::OnStop()
{
	scenePool.GetRootPtr()->OnStop();

	RE_INPUT->PauseEvents();
	scenePool.ClearPool();
	scenePool.InsertPool(&savedState);
	savedState.ClearPool();
	RE_EDITOR->SetSelected(0);
	SetupScene();
	RE_INPUT->ResumeEvents();

	is_playing = false;
}

void ModuleScene::RecieveEvent(const Event& e)
{
	switch (e.type)
	{
	case RE_EventType::GO_CHANGED_TO_ACTIVE:
	{
		RE_GameObject* go = scenePool.GetGOPtr(e.data1.AsUInt64());
		go->ResetGOandChildsAABB();

		for (auto draw_go : go->GetActiveDrawableGOandChildsPtr())
			(draw_go->IsStatic() ? static_tree : dynamic_tree).PushNode(draw_go->GetUID(), draw_go->GetGlobalBoundingBox());

		haschanges = true;
		break;
	}
	case RE_EventType::GO_CHANGED_TO_INACTIVE:
	{
		for (auto draw_go : scenePool.GetGOPtr(e.data1.AsUInt64())->GetActiveDrawableGOandChildsPtr())
			(draw_go->IsStatic() ? static_tree : dynamic_tree).PopNode(draw_go->GetUID());

		haschanges = true;
		break;
	}
	case RE_EventType::GO_CHANGED_TO_STATIC:
	{
		GO_UID go_uid = e.data1.AsUInt64();
		RE_GameObject* go = scenePool.GetGOPtr(go_uid);
		if (go->IsActive())
		{
			dynamic_tree.PopNode(go_uid);
			static_tree.PushNode(go_uid, go->GetGlobalBoundingBox());
		}

		haschanges = true;
		break;
	}
	case RE_EventType::GO_CHANGED_TO_NON_STATIC:
	{
		GO_UID go_uid = e.data1.AsUInt64();
		RE_GameObject* go = scenePool.GetGOPtr(go_uid);
		if (go->IsActive())
		{
			static_tree.PopNode(go_uid);
			dynamic_tree.PushNode(go_uid, go->GetGlobalBoundingBox());
		}

		haschanges = true;
		break;
	}
	case RE_EventType::GO_HAS_NEW_CHILD:
	{
		RE_GameObject* child_go = scenePool.GetGOPtr(e.data2.AsUInt64());
		child_go->ResetGOandChildsAABB();
		child_go->OnTransformModified();

		eastl::vector<RE_GameObject*> all = child_go->GetActiveDrawableGOandChildsPtr();
		for (auto draw_go : all) (draw_go->IsStatic() ? static_tree : dynamic_tree).PushNode(draw_go->GetUID(), draw_go->GetGlobalBoundingBox());
		for (auto draw_go : all) (draw_go->IsStatic() ? static_tree : dynamic_tree).UpdateNode(draw_go->GetUID(), draw_go->GetGlobalBoundingBox());

		haschanges = true;
		break;
	}
	case RE_EventType::DESTROY_GO:
	{
		GO_UID go_uid = e.data1.AsUInt64();
		RE_GameObject* go = scenePool.GetGOPtr(go_uid);

		if (go->IsActive())
			for (auto draw_go : go->GetActiveDrawableGOandChildsCPtr())
				(draw_go->IsStatic() ? static_tree : dynamic_tree).PopNode(draw_go->GetUID());

		to_delete.push(go_uid);
		haschanges = true;
		break;
	}
	case RE_EventType::TRANSFORM_MODIFIED:
	{
		RE_GameObject* go = scenePool.GetGOPtr(e.data1.AsUInt64());
		if (go->IsActive())
		{
			go->ResetGOandChildsAABB();
			go->OnTransformModified();

			for (auto draw_go : go->GetActiveDrawableGOandChildsCPtr())
				(draw_go->IsStatic() ? static_tree : dynamic_tree).UpdateNode(draw_go->GetUID(), draw_go->GetGlobalBoundingBox());
		}

		haschanges = true;
		break;
	}
	case RE_EventType::PLANE_CHANGE_TO_MESH:
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

void ModuleScene::CreatePrimitive(ComponentType type, const GO_UID parent)
{
	scenePool.AddGO("Primitive", Validate(parent), true)->AddNewComponent(type);
}

void ModuleScene::CreateCamera(const GO_UID parent)
{
	scenePool.AddGO("Camera", Validate(parent), true)->AddNewComponent(C_CAMERA);
}

void ModuleScene::CreateLight(const GO_UID parent)
{
	RE_GameObject* light_go = scenePool.AddGO("Light", Validate(parent), true);

	dynamic_cast<RE_CompPrimitive*>(light_go->AddNewComponent(C_SPHERE))->SetColor(
		dynamic_cast<RE_CompLight*>(light_go->AddNewComponent(C_LIGHT))->diffuse);
}

void ModuleScene::CreateMaxLights(const GO_UID parent)
{
	GO_UID container_go = scenePool.AddGO("Bunch of Lights", (parent) ? parent : GetRootUID(), true)->GetUID();

	eastl::string name = "light ";
	for (int x = 0; x < 8; ++x)
	{
		for (int y = 0; y < 8; ++y)
		{
			RE_GameObject* light_go = scenePool.AddGO((name + eastl::to_string(x) + "x" + eastl::to_string(y)).c_str(), container_go);
			light_go->GetTransformPtr()->SetPosition(math::vec((static_cast<float>(x) * 12.5f) - 50.f, 0.f, (static_cast<float>(y) * 12.5f) - 50.f));

			static math::vec colors[5] = { math::vec(1.f,0.f,0.f), math::vec(0.f,1.f,0.f), math::vec(0.f,0.f,1.f), math::vec(1.f,1.f,0.f), math::vec(0.f,1.f,1.f) };
			dynamic_cast<RE_CompPrimitive*>(light_go->AddNewComponent(C_SPHERE))->SetColor(
				dynamic_cast<RE_CompLight*>(light_go->AddNewComponent(C_LIGHT))->diffuse = colors[RE_MATH->RandomInt() % 5]);
		}
	}
}

void ModuleScene::CreateWater(const GO_UID parent)
{
	RE_GameObject* water_go = scenePool.AddGO("Water", Validate(parent), true);
	water_go->AddNewComponent(C_WATER)->UseResources();
	RE_CompTransform* transform = water_go->GetTransformPtr();
	transform->SetRotation({ -(math::pi / 2.f), 0.0f, 0.0f });
	transform->SetScale({ 10.0f, 10.0f, 1.0f });
}

void ModuleScene::CreateParticleSystem(const GO_UID parent)
{
	RE_GameObject* go = scenePool.AddGO("Particle System", Validate(parent), true);
	go->SetStatic(false, false);
	dynamic_cast<RE_CompParticleEmitter*>(go->AddNewComponent(C_PARTICLEEMITER))->UseResources();
}

void ModuleScene::AddGOPool(RE_ECS_Pool* toAdd)
{
	toAdd->UseResources();
	GO_UID justAdded = scenePool.InsertPool(toAdd, true);
	RE_EDITOR->SetSelected(justAdded);
	haschanges = true;
}

GO_UID ModuleScene::RayCastGeometry(math::Ray & global_ray) const
{
	GO_UID ret = 0;

	eastl::queue<GO_UID> aabb_collisions;
	static_tree.CollectIntersections(global_ray, aabb_collisions);
	dynamic_tree.CollectIntersections(global_ray, aabb_collisions);

	eastl::vector<eastl::pair<GO_UID,const RE_GameObject*>> gos;
	while (!aabb_collisions.empty())
	{
		GO_UID id = aabb_collisions.front();
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

void ModuleScene::FustrumCulling(eastl::vector<const RE_GameObject*>& container, const math::Frustum & frustum) const
{
	eastl::queue<GO_UID> goIndex;
	static_tree.CollectIntersections(frustum, goIndex);
	dynamic_tree.CollectIntersections(frustum, goIndex);

	while (!goIndex.empty())
	{
		container.push_back(scenePool.GetGOCPtr(goIndex.front()));
		goIndex.pop();
	}
}

void ModuleScene::SaveScene(const char* newName)
{
	RE_Scene* scene = (unsavedScene) ? unsavedScene : dynamic_cast<RE_Scene*>(RE_RES->At(currentScene));

	RE_INPUT->PauseEvents();

	scene->SetName((unsavedScene) ? (newName) ? newName : GetRootCPtr()->name.c_str() : scene->GetName());
	scene->Save(&scenePool);
	scene->SaveMeta();

	if (unsavedScene)
	{
		currentScene = RE_RES->Reference(scene);
		RE_RENDER->PushThumnailRend(currentScene);
		unsavedScene = nullptr;
	}
	else
		RE_RENDER->PushThumnailRend(scene->GetMD5(), true);

	haschanges = false;
	RE_INPUT->ResumeEvents();
}

const char* ModuleScene::GetCurrentScene() const { return currentScene; }

void ModuleScene::ClearScene()
{
	RE_INPUT->PauseEvents();

	if (GetRootUID()) scenePool.UnUseResources();
	savedState.ClearPool();

	static_tree.Clear();
	dynamic_tree.Clear();
	scenePool.ClearPool();

	RE_GameObject* root = scenePool.AddGO("root", 0);
	root->SetStatic(false);

	GO_UID root_uid = scenePool.GetRootUID();
	scenePool.AddGO("Camera", root_uid)->AddNewComponent(C_CAMERA);
	RE_EDITOR->SetSelected(root_uid);

	RE_CameraManager::RecallSceneCameras();

	RE_INPUT->ResumeEvents();
}

void ModuleScene::NewEmptyScene(const char* name)
{
	RE_INPUT->PauseEvents();
	RE_EDITOR->ClearCommands();

	if (unsavedScene) { DEL(unsavedScene); }
	else currentScene = nullptr;

	unsavedScene = new RE_Scene();
	unsavedScene->SetName(name);
	unsavedScene->SetType(R_SCENE);

	scenePool.UnUseResources();
	savedState.ClearPool();
	scenePool.ClearPool();

	scenePool.AddGO("root", 0)->SetStatic(false);

	SetupScene();
	RE_EDITOR->SetSelected(0);
	RE_INPUT->ResumeEvents();

	haschanges = false;
}

void ModuleScene::LoadScene(const char* sceneMD5, bool ignorehandle)
{
	RE_INPUT->PauseEvents();
	RE_EDITOR->ClearCommands();
	if (unsavedScene) DEL(unsavedScene);
	
	scenePool.UnUseResources();
	savedState.ClearPool();
	scenePool.ClearPool();

	RE_LOG("Loading scene from own format:");
	if(!ignorehandle) RE_LOGGER.ScopeProcedureLogging();
	RE_Timer timer;
	currentScene = sceneMD5;
	RE_Scene* scene = dynamic_cast<RE_Scene*>(RE_RES->At(currentScene));
	RE_RES->Use(sceneMD5);

	RE_ECS_Pool* loadedDO = scene->GetPool();
	if (loadedDO)scenePool.InsertPool(loadedDO);
	else RE_LOG_ERROR("Can't Load Scene");

	DEL(loadedDO);

	RE_RES->UnUse(sceneMD5);
	scenePool.UseResources();
	SetupScene();
	RE_EDITOR->SetSelected(0);

	RE_LOG("Time loading scene: %u ms", timer.Read());
	if (!ignorehandle) RE_LOGGER.EndScope();
	RE_INPUT->ResumeEvents();
}

void ModuleScene::SetupScene()
{
	RE_INPUT->PauseEvents();

	// Render Camera Management
	RE_CameraManager::RecallSceneCameras();
	if (!RE_CameraManager::HasMainCamera())
	{
		RE_INPUT->ResumeEvents();
		CreateCamera(scenePool.GetRootUID());
		RE_INPUT->PauseEvents();
	}

	savedState.ClearPool();
	savedState.InsertPool(&scenePool);

	// Setup Tree AABBs¡
	static_tree.Clear();
	dynamic_tree.Clear();
	GetRootPtr()->ResetGOandChildsAABB();

	eastl::vector<eastl::pair<const GO_UID, RE_GameObject*>> gos = scenePool.GetAllGOData();
	for (unsigned int i = 0; i < gos.size(); i++)
	{
		if (gos[i].second->HasActiveRenderGeo())
			(gos[i].second->IsStatic() ? static_tree : dynamic_tree).PushNode(gos[i].first, gos[i].second->GetGlobalBoundingBox());
	}

	RE_INPUT->ResumeEvents();
}

