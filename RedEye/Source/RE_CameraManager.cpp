#include "RE_CameraManager.h"
#include "RE_CompCamera.h"
#include "Application.h"
#include "ModuleScene.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include <EASTL/stack.h>

RE_CompCamera* RE_CameraManager::editor_camera = nullptr;
RE_CompCamera* RE_CameraManager::main_camera = nullptr;

RE_CameraManager::RE_CameraManager() {}
RE_CameraManager::~RE_CameraManager() {}

void RE_CameraManager::Init()
{
	editor_camera = new RE_CompCamera();
	editor_camera->SetUp(nullptr);
	editor_camera->GetTransform()->SetPosition(math::vec(0.f, 5.f, -5.f));
}

void RE_CameraManager::Clear()
{
	DEL(editor_camera);
	scene_cameras.clear();
}

RE_CompCamera * RE_CameraManager::CurrentCamera()
{
	RE_CompCamera * ret = editor_camera;
	if (App::GetState() != GS_STOP && main_camera) ret = main_camera;
	return ret;
}

RE_CompCamera * RE_CameraManager::EditorCamera() { return editor_camera; }
RE_CompCamera* RE_CameraManager::MainCamera() { return main_camera; }
bool RE_CameraManager::HasMainCamera() { return main_camera != nullptr; }

void RE_CameraManager::OnWindowChangeSize(float width, float height)
{
	// Adapt cameras to knew window dimensions
	// editor_camera->SetBounds(width, height);
	for (auto cam : scene_cameras)
		cam->SetBounds(width, height);
}

void RE_CameraManager::AddMainCamera(RE_CompCamera* cam)
{
	if (scene_cameras.empty()) main_camera = cam;
	scene_cameras.push_back(cam);
}

eastl::list<RE_CompCamera*> RE_CameraManager::GetCameras() const { return scene_cameras; }

void RE_CameraManager::RecallCameras(const RE_GameObject * root)
{
	scene_cameras.clear();

	eastl::stack<const RE_GameObject*> gos;
	gos.push(root);
	while (!gos.empty())
	{
		const RE_GameObject * go = gos.top();
		RE_CompCamera * cam = go->GetCamera();

		if (cam != nullptr)
			scene_cameras.push_back(cam);

		gos.pop();

		for (auto child : go->GetChilds())
			gos.push(child);
	}

	main_camera = nullptr;
	if (!scene_cameras.empty())
	{
		for (eastl::list<RE_CompCamera*>::iterator cam = scene_cameras.begin();
			cam != scene_cameras.end(); cam++)
		{
			if (!main_camera) main_camera = (*cam);

			/* TODO:
			if ((*cam)->isMain)
			else: more than 1 main cameras*/
		}
	}
}

const math::Frustum RE_CameraManager::GetCullingFrustum() const
{
	for (auto cam : scene_cameras)
		if (cam->OverridesCulling())
			return cam->GetFrustum();

	return editor_camera->GetFrustum();
}

