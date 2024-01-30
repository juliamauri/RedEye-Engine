#include "RE_CameraManager.h"

#include "Application.h"
#include "ModuleScene.h"

#include <EASTL/list.h>
#include <EASTL/stack.h>

COMP_UID main_camera = 0;
bool cull_scene = true;

RE_CompCamera* RE_CameraManager::MainCamera()
{
	if (!HasMainCamera()) return nullptr;
	return dynamic_cast<RE_CompCamera*>(RE_SCENE->GetCScenePool()->GetComponentPtr(main_camera, RE_Component::Type::CAMERA));
}
void RE_CameraManager::SetAsMainCamera(COMP_UID id) { main_camera = id; }
bool RE_CameraManager::HasMainCamera() { return main_camera != 0; }

void RE_CameraManager::OnWindowChangeSize(math::float2 window_bounds)
{
	for (auto cam : RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::CAMERA))
		dynamic_cast<RE_CompCamera*>(cam)->Camera.SetBounds(window_bounds);
}

void RE_CameraManager::RecallSceneCameras()
{
	main_camera = 0;
	auto cams = RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::CAMERA);
	if (!cams.empty()) main_camera = dynamic_cast<RE_CompCamera*>(cams[0])->GetPoolID();
}

const math::Frustum* RE_CameraManager::GetCullingFrustum()
{
	for (auto cam : RE_SCENE->GetScenePool()->GetAllCompCPtr(RE_Component::Type::CAMERA))
	{
		auto camera = dynamic_cast<const RE_CompCamera*>(cam);
		if (camera->override_cull) return &camera->Camera.GetFrustum();
	}

	return nullptr;
}
