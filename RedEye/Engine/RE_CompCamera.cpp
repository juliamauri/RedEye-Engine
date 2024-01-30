#include "RE_CompCamera.h"

#include "RE_CompTransform.h"
#include "RE_ECS_Pool.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>

void RE_CompCamera::SetProperties(
	math::float2 bounds,
	AspectRatio ar,
	float v_fov,
	float n_plane,
	float f_plane,
	bool _usingSkybox,
	const char* _skyboxMD5,
	bool _draw_frustum)
{
	Camera.SetupFrustum(bounds, ar, v_fov, n_plane, f_plane);
	Camera.SetSkyBox(_skyboxMD5, _usingSkybox);
	draw_frustum = _draw_frustum;
	need_recalculation = true;
}

void RE_CompCamera::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent)
{
	pool_gos = pool;
	go = parent;
	pool_gos->AtPtr(go)->ReportComponent(id, type);
	Camera = dynamic_cast<const RE_CompCamera*>(copy)->Camera;
}

void RE_CompCamera::Update()
{
	if (!need_recalculation) return;

	math::float4x4 trs = GetTransform()->GetGlobalMatrix();
	Camera.SetFrame(trs.Row3(3), -trs.WorldZ(), trs.WorldY());
	need_recalculation = false;
}

void RE_CompCamera::DrawProperties()
{
	if (!ImGui::CollapsingHeader("Camera")) return;

	ImGui::Checkbox("Draw Frustum", &draw_frustum);
	ImGui::Checkbox("Override Culling", &override_cull);
	ImGui::Separator();
	Camera.DrawProperties();
}

RE_CompTransform* RE_CompCamera::GetTransform() const { return GetGOCPtr()->GetTransformPtr(); }

eastl::vector<const char*> RE_CompCamera::GetAllResources() const
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
	node->Push("override_cull", override_cull);
	DEL(node)
}

void RE_CompCamera::JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources)
{
	Camera.JsonDeserialize(node->PullJObject("Camera"), resources);
	draw_frustum = node->PullBool("draw_frustum", false);
	draw_frustum = node->PullBool("override_cull", false);
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