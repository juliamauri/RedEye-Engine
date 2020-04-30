#ifndef __CAMERAMANAGER_H__
#define __CAMERAMANAGER_H__

#include "RE_CompCamera.h"
#include "MathGeoLib/include/Geometry/Frustum.h"
#include <EASTL/list.h>

class RE_GameObject;
	
class RE_CameraManager
{
public:

	RE_CameraManager();
	~RE_CameraManager();

	void Init();
	void Clear();

	static RE_CompCamera * CurrentCamera();
	static RE_CompCamera * EditorCamera();
	static RE_CompCamera * MainCamera();
	static bool HasMainCamera();

	void OnWindowChangeSize(float width, float height);
	void AddMainCamera(RE_CompCamera* cam);
	void RecallCameras(const RE_GameObject * root);

	eastl::list<RE_CompCamera*> GetCameras() const;
	const math::Frustum GetCullingFrustum() const;

private:

	static RE_CompCamera* editor_camera;
	static RE_CompCamera* main_camera;

	eastl::list<RE_CompCamera*> scene_cameras;
	bool cull_scene = true;
};

#endif // !__CAMERAMANAGER_H__