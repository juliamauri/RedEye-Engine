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

void RE_CompCamera::SetProperties(
	float n_plane,
	float f_plane,
	float v_fov,
	RE_Camera::AspectRatio ar,
	bool _usingSkybox,
	const char* _skyboxMD5,
	bool _draw_frustum)
{
	Camera.SetupFrustum(n_plane, f_plane, ar, v_fov);
	Camera.SetSkyBox(_skyboxMD5, _usingSkybox);
	draw_frustum = _draw_frustum;
	need_recalculation = true;
}

void RE_CompCamera::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent)
{
	pool_gos = pool;
	go = parent;
	pool_gos->AtPtr(go)->ReportComponent(id, type);

	auto cmpCamera = dynamic_cast<const RE_CompCamera*>(copy);
	SetProperties(
		cmpCamera->Camera.GetNearPlane(),
		cmpCamera->Camera.GetFarPlane(),
		cmpCamera->Camera.GetVFOVRads(),
		cmpCamera->Camera.GetAspectRatio(),
		cmpCamera->Camera.isUsingSkybox(),
		cmpCamera->Camera.GetSkybox(),
		cmpCamera->draw_frustum);
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
	}
}

RE_CompTransform* RE_CompCamera::GetTransform() const
{
	return GetGOCPtr()->GetTransformPtr();
}

void RE_CompCamera::UseResources() { Camera.UseSkybox(); }
void RE_CompCamera::UnUseResources() { Camera.UnUseSkybox(); }

eastl::vector<const char*> RE_CompCamera::GetAllResources()
{
	eastl::vector<const char*> ret;
	const char* skybox = Camera.GetSkybox();
	if (skybox) ret.push_back(skybox);
	return ret;
}

#pragma region Serialization

void RE_CompCamera::JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	Camera.JsonSerialize(node->PushJObject("Camera"), resources);
	node->Push("draw_frustum", draw_frustum);
	DEL(node)
}

void RE_CompCamera::JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources)
{
	Camera.JsonDeserialize(node->PullJObject("Camera"), resources);
	draw_frustum = node->PullBool("draw_frustum", false);
	DEL(node)
}

size_t RE_CompCamera::GetBinarySize() const
{
	return Camera.GetBinarySize() + sizeof(bool) * 2;
}

void RE_CompCamera::BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const
{
	Camera.BinarySerialize(cursor, resources);

	size_t size = sizeof(bool);
	memcpy(cursor, &draw_frustum, size);
	cursor += size;

	memcpy(cursor, &override_cull, size);
	cursor += size;
}

void RE_CompCamera::BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources)
{
	Camera.BinaryDeserialize(cursor, resources);

	size_t size = sizeof(bool);
	memcpy(&draw_frustum, cursor, size);
	cursor += size;

	memcpy(&override_cull, cursor, size);
	cursor += size;
}

#pragma endregion