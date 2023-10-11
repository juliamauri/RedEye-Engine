#ifndef __CAMERA_MANAGER_H__
#define __CAMERA_MANAGER_H__
	
#include "RE_CompCamera.h"

namespace RE_CameraManager
{
	RE_CompCamera * MainCamera();
	void SetAsMainCamera(COMP_UID id);
	bool HasMainCamera();

	void OnWindowChangeSize(float width, float height);
	void RecallSceneCameras();

	const math::Frustum* GetCullingFrustum();
};

#endif // !__CAMERA_MANAGER_H__