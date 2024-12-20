#include <MGL/Geometry/Frustum.h>
#include <EASTL/list.h>

#include "RE_CameraManager.h"

#include "RE_Memory.h"
#include "RE_Time.h"
#include "RE_CompCamera.h"
#include "Application.h"
#include "ModuleScene.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include <EASTL/stack.h>

RE_CompCamera* RE_CameraManager::editor_camera = nullptr;
RE_CompCamera* RE_CameraManager::particleEditor_camera = nullptr;
COMP_UID RE_CameraManager::main_camera = 0ull;

void RE_CameraManager::Init()
{
	editor_camera = new RE_CompCamera();
	editor_camera->SetParent(0ull);
	editor_camera->SetProperties();
	editor_camera->GetTransform()->SetPosition(math::vec(0.f, 5.f, -5.f));

	particleEditor_camera = new RE_CompCamera();
	particleEditor_camera->SetParent(0ull);
	particleEditor_camera->SetProperties();
	particleEditor_camera->GetTransform()->SetPosition(math::vec(0.f, 0.f, 10.f));
}

void RE_CameraManager::Clear()
{
	DEL(editor_camera)
	DEL(particleEditor_camera)
}

RE_CompCamera* RE_CameraManager::CurrentCamera()
{
	return main_camera ?
		dynamic_cast<RE_CompCamera*>(RE_SCENE->GetScenePool()->GetComponentPtr(main_camera, RE_Component::Type::CAMERA)) :
		editor_camera;
}

RE_CompCamera* RE_CameraManager::EditorCamera() { return editor_camera; }
RE_CompCamera* RE_CameraManager::ParticleEditorCamera() { return particleEditor_camera; }
RE_CompCamera* RE_CameraManager::MainCamera() { return dynamic_cast<RE_CompCamera*>(RE_SCENE->GetScenePool()->GetComponentPtr(main_camera, RE_Component::Type::CAMERA)); }
bool RE_CameraManager::HasMainCamera() { return main_camera != 0ull; }

void RE_CameraManager::OnWindowChangeSize(float width, float height)
{
	// Adapt cameras to knew window dimensions
	// editor_camera->SetBounds(width, height);
	for (auto cam : RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::CAMERA))
		dynamic_cast<RE_CompCamera*>(cam)->SetBounds(width, height);
}

void RE_CameraManager::AddMainCamera(RE_CompCamera* cam)
{
	main_camera = cam->GetPoolID();
}

void RE_CameraManager::RecallSceneCameras()
{
	main_camera = 0ull;
	eastl::vector<RE_Component*> allCameras = RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::CAMERA);

	for (auto cam : allCameras)
	{
		if (!main_camera) main_camera = dynamic_cast<RE_CompCamera*>(cam)->GetPoolID();

		/* TODO Rub: handle several main cameras
		if ((*cam)->isMain)
		else: more than 1 main cameras*/
	}
}

const math::Frustum RE_CameraManager::GetCullingFrustum() const
{
	for (auto cam : RE_SCENE->GetScenePool()->GetAllCompCPtr(RE_Component::Type::CAMERA))
	{
		const RE_CompCamera* camera = dynamic_cast<const RE_CompCamera*>(cam);

		if (camera->OverridesCulling())
			return camera->GetFrustum();
	}

	return editor_camera->GetFrustum();
}

