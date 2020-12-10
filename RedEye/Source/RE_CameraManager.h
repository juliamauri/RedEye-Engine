#ifndef __CAMERAMANAGER_H__
#define __CAMERAMANAGER_H__

#include "Globals.h"
#include "RE_Math.h"

class RE_CompCamera;
	
namespace RE_CameraManager
{
	void Init();
	void Clear();

	RE_CompCamera* CurrentCamera();
	RE_CompCamera* EditorCamera();
	RE_CompCamera* MainCamera();
	bool HasMainCamera();

	void OnWindowChangeSize(float width, float height);
	void AddMainCamera(RE_CompCamera* cam);
	void RecallSceneCameras();

	const math::Frustum GetCullingFrustum();

	namespace Internal
	{
		static RE_CompCamera* editor_camera = nullptr;
		static UID main_camera = 0;
		static bool cull_scene = true;
	};
};

#endif // !__CAMERAMANAGER_H__