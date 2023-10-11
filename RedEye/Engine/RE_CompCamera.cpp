#include "RE_CompCamera.h"

#include "RE_Memory.h"
#include "RE_Math.h"
#include "Application.h"
#include "RE_Json.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_ShaderImporter.h"
#include "RE_CompTransform.h"
#include "RE_ECS_Pool.h"
#include "RE_SkyBox.h"

#include <ImGui/imgui.h>
#include <SDL2/SDL_opengl.h>

void RE_CompCamera::SetProperties(float n_plane, float f_plane, float v_fov, RE_Camera::AspectRatio ar, bool _draw_frustum, bool _usingSkybox, const char* _skyboxMD5)
{
	Camera.SetupFrustum(n_plane, f_plane, ar, v_fov);

	draw_frustum = _draw_frustum;
	usingSkybox = _usingSkybox;
	skyboxMD5 = _skyboxMD5;
	need_recalculation = true;
}

void RE_CompCamera::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent)
{
	pool_gos = pool;
	go = parent;
	pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompCamera* cmpCamera = dynamic_cast<RE_CompCamera*>(copy);
	SetProperties(
		cmpCamera->Camera.GetNearPlane(),
		cmpCamera->Camera.GetFarPlane(),
		cmpCamera->Camera.GetVFOVRads(),
		cmpCamera->Camera.GetAspectRatio(),
		cmpCamera->draw_frustum,
		cmpCamera->usingSkybox,
		cmpCamera->skyboxMD5);
}

void RE_CompCamera::Update()
{
	if (need_recalculation)
	{
		math::float4x4 trs = GetTransform()->GetGlobalMatrix();
		Camera.SetFrame(trs.Row3(3), -trs.WorldZ(), trs.WorldY());
		//global_view = frustum.ViewMatrix().Transposed();
		//global_projection = frustum.ProjectionMatrix().Transposed();
		need_recalculation = false;
	}
}

void RE_CompCamera::OnTransformModified() { need_recalculation = true; }

void RE_CompCamera::DrawProperties()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::Checkbox("Draw Frustum", &draw_frustum);
		ImGui::Checkbox("Override Culling", &override_cull);

		if (Camera.DrawProperties())
			RE_INPUT->Push(RE_EventType::UPDATE_SCENE_WINDOWS, RE_EDITOR, go);

		ImGui::Separator();
		ImGui::Checkbox("Use skybox", &usingSkybox);

		if (usingSkybox)
		{
			RE_SkyBox* skyRes = nullptr;

			if (skyboxMD5)
			{
				skyRes = dynamic_cast<RE_SkyBox*>(RE_RES->At(skyboxMD5));
				if (ImGui::Button(skyRes->GetName())) RE_RES->PushSelected(skyRes->GetMD5(), true);
				ImGui::SameLine();
				if (ImGui::Button("Back to Default Skybox")) skyboxMD5 = nullptr;
			}
			else
			{
				ImGui::Text("This component camera is using the default skybox.");
				skyRes = dynamic_cast<RE_SkyBox*>(RE_RES->At(RE_InternalResources::GetDefaultSkyBox()));
				if (ImGui::Button(skyRes->GetName())) RE_RES->PushSelected(skyRes->GetMD5(), true);
			}

			if (ImGui::BeginMenu("Change skybox"))
			{
				eastl::vector<ResourceContainer*> materials = RE_RES->GetResourcesByType(ResourceContainer::Type::SKYBOX);
				bool none = true;
				for (auto material : materials)
				{
					if (material->isInternal()) continue;

					none = false;
					if (ImGui::MenuItem(material->GetName()))
					{
						if (skyboxMD5) RE_RES->UnUse(skyboxMD5);
						skyboxMD5 = material->GetMD5();
						if (skyboxMD5) RE_RES->Use(skyboxMD5);
					}
				}
				if (none) ImGui::Text("No custom skyboxes on assets");
				ImGui::EndMenu();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#SkyboxReference"))
				{
					if (skyboxMD5) RE_RES->UnUse(skyboxMD5);
					skyboxMD5 = *static_cast<const char**>(dropped->Data);
					if (skyboxMD5) RE_RES->Use(skyboxMD5);
				}
				ImGui::EndDragDropTarget();
			}
		}
	}
}

RE_CompTransform* RE_CompCamera::GetTransform() const
{
	return GetGOCPtr()->GetTransformPtr();
}

void RE_CompCamera::DrawSkybox() const
{
	RE_SkyBox* skyRes = dynamic_cast<RE_SkyBox*>(RE_RES->At(skyboxMD5 ? skyboxMD5 : RE_InternalResources::GetDefaultSkyBox()));
	if (skyRes) skyRes->DrawSkybox();
}

bool RE_CompCamera::isUsingSkybox() const { return usingSkybox; }
const char* RE_CompCamera::GetSkybox() const { return skyboxMD5; }
void RE_CompCamera::DeleteSkybox() { usingSkybox = false; skyboxMD5 = nullptr; }
void RE_CompCamera::SetSkyBox(const char* md5) { skyboxMD5 = md5; }
void RE_CompCamera::UseResources() { if (skyboxMD5) RE_RES->Use(skyboxMD5); }
void RE_CompCamera::UnUseResources() { if (skyboxMD5) RE_RES->UnUse(skyboxMD5); }

eastl::vector<const char*> RE_CompCamera::GetAllResources()
{
	eastl::vector<const char*> ret;
	if (skyboxMD5) ret.push_back(skyboxMD5);
	return ret;
}

#pragma region Serialization

void RE_CompCamera::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	Camera.JsonSerialize(node->PushJObject("Camera"));

	node->Push("draw_frustum", draw_frustum);
	node->Push("usingSkybox", usingSkybox);
	node->Push("skyboxResource", (skyboxMD5) ? resources->at(skyboxMD5) : -1);
}

void RE_CompCamera::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
	Camera.JsonDeserialize(node->PullJObject("Camera"));

	draw_frustum = node->PullBool("draw_frustum", false);
	usingSkybox = node->PullBool("usingSkybox", true);
	int sbRes = node->PullInt("skyboxResource", -1);
	skyboxMD5 = (sbRes != -1) ? resources->at(sbRes) : nullptr;

	DEL(node)
}

size_t RE_CompCamera::GetBinarySize() const
{
	return Camera.GetBinarySize() + sizeof(bool) * 2 + sizeof(int);
}

void RE_CompCamera::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	Camera.BinarySerialize(cursor);

	size_t size = sizeof(bool);
	memcpy(cursor, &draw_frustum, size);
	cursor += size;

	memcpy(cursor, &usingSkybox, size);
	cursor += size;

	size = sizeof(int);
	int sbres = (skyboxMD5) ? resources->at(skyboxMD5) : -1;
	memcpy(cursor, &sbres, size);
	cursor += size;
}

void RE_CompCamera::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	Camera.BinaryDeserialize(cursor);

	size_t size = sizeof(bool);
	memcpy(&draw_frustum, cursor, size);
	cursor += size;

	memcpy(&usingSkybox, cursor, size);
	cursor += size;

	size = sizeof(int);
	int sbRes = -1;
	memcpy(&sbRes, cursor, size);
	skyboxMD5 = (sbRes != -1) ? resources->at(sbRes) : nullptr;
	cursor += size;
}

#pragma endregion