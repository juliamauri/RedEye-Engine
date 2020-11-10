#include "RE_CameraManager.h"
#include "RE_CompCamera.h"
#include "Application.h"
#include "ModuleScene.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include <EASTL/stack.h>

RE_CompCamera* RE_CameraManager::editor_camera = nullptr;
UID RE_CameraManager::main_camera = 0ull;

RE_CameraManager::RE_CameraManager() {}
RE_CameraManager::~RE_CameraManager() {}

void RE_CameraManager::Init()
{
	editor_camera = new RE_CompCamera();
	editor_camera->SetProperties();
	editor_camera->GetTransform()->SetPosition(math::vec(0.f, 5.f, -5.f));
}

void RE_CameraManager::Clear()
{
	DEL(editor_camera);
}

RE_CompCamera * RE_CameraManager::CurrentCamera()
{
	RE_CompCamera * ret = editor_camera;
	if (App::GetState() != GS_STOP && main_camera) ret = static_cast<RE_CompCamera*>(ModuleScene::GetScenePool()->GetComponentPtr(main_camera, C_CAMERA));
	return ret;
}

RE_CompCamera * RE_CameraManager::EditorCamera() { return editor_camera; }
RE_CompCamera* RE_CameraManager::MainCamera() { return static_cast<RE_CompCamera*>(ModuleScene::GetScenePool()->GetComponentPtr(main_camera, C_CAMERA)); }
bool RE_CameraManager::HasMainCamera() { return main_camera != 0ull; }

void RE_CameraManager::OnWindowChangeSize(float width, float height)
{
	// Adapt cameras to knew window dimensions
	// editor_camera->SetBounds(width, height);
	for (auto cam : ModuleScene::GetScenePool()->GetAllCompPtr(C_CAMERA))
		static_cast<RE_CompCamera*>(cam)->SetBounds(width, height);
}

void RE_CameraManager::AddMainCamera(RE_CompCamera* cam)
{
	if (ModuleScene::GetScenePool()->GetAllCompPtr(C_CAMERA).empty()) main_camera = cam->GetPoolID();
}

void RE_CameraManager::RecallSceneCameras()
{
	main_camera = 0ull;
	eastl::vector<RE_Component*> allCameras = ModuleScene::GetScenePool()->GetAllCompPtr(C_CAMERA);

	for (auto cam : allCameras)
	{
		if (!main_camera) main_camera = static_cast<RE_CompCamera*>(cam)->GetPoolID();

		/* TODO Rub:
		if ((*cam)->isMain)
		else: more than 1 main cameras*/
	}
}

const math::Frustum RE_CameraManager::GetCullingFrustum() const
{
	for (auto cam : ModuleScene::GetScenePool()->GetAllCompCPtr(C_CAMERA)) {
		const RE_CompCamera* camera = static_cast<const RE_CompCamera*>(cam);

		if (camera->OverridesCulling())
			return camera->GetFrustum();
	}

	return editor_camera->GetFrustum();
}

