#ifndef __RE_CAMERA_H__
#define __RE_CAMERA_H__

#include "RE_Math.h"

class RE_Camera
{
public:
	//Camera initialize world origin at (0, 0, 0) and planes distance 0.1f near and 100.0f far
	//@camraType -> true = Prespective | false = Orthographic
	//@near and @far -> distances of planes from camera position
	RE_Camera(bool cameraType, float near_plane = 0.1f, float far_plane = 100.0f);

	//Set the camera position, must use this
	//@pos ->camera position
	void SetPos(math::float3 pos);

	//Set World origin Position
	//@world -> world position
	void SetWorldOrigin(math::float3 world);

	//Set the position planes by distance
	//@near and @far -> distances of planes from camera position
	void SetPlanesDistance(float near_plane, float far_plane);

	//If window size changed, needed to call this function.
	void ResetFov();

	//prespective to orthograohic or inverse
	void ChangeCameraType();

	//Get camera view matrix
	math::float4x4 GetView();

	//Get camera projection matrix
	math::float4x4 GetProjection();

private:
	//Camera frustum
	math::Frustum camera;

	//true = Prespective | false = Orthographic
	bool cameraType;
};

#endif // !__RE_CAMERA_H__