#ifndef __RE_COMPCAMERA_H__
#define __RE_COMPCAMERA_H__

#include "RE_Component.h"
#include "RE_Math.h"

class RE_CompTransform;

enum CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class RE_CompCamera : public RE_Component
{
public:
	//Camera initialize world origin at (0, 0, 0) and planes distance 0.1f near and 100.0f far
	//@param camraType -> true = Prespective | false = Orthographic
	//@param near and @far -> distances of planes from camera position
	RE_CompCamera(RE_GameObject* go = nullptr, bool cameraType = true, float near_plane = 0.1f, float far_plane = 100.0f);
	
	void PreUpdate() override;
	void Update() override;
	void PostUpdate() override;

	void Draw() override;

	void DrawProperties() override;

	void OnTransformModified() override;

	//Set the position planes by distance
	//@param near and @far -> distances of planes from camera position
	void SetPlanesDistance(float near_plane, float far_plane);

	//If window size changed, needed to call this function.
	void ResetFov();

	//prespective to orthograohic or inverse
	void ChangeCameraType();

	void SetPos(math::vec position);

	//Get camera view matrix
	math::float4x4 GetView();

	//Get camera projection matrix
	math::float4x4 GetProjection();

	void SetEulerAngle(float pitch, float yaw);

	//LookAt, return the view matrix (view * transformation matrix)
	//@param cameraTarget -> The object that the camera looks
	void LookAt(math::vec cameraTarget);

	float* RealView();

	//Move camera
	void Move(CameraMovement dir, float speed);

private:
	//transform
	RE_CompTransform* transform = nullptr;

	//Camera frustum
	math::Frustum camera;

	math::float4x4 view;

	//true = Prespective | false = Orthographic
	bool cameraType;

	float yaw = 0.0f;
	float pitch = 0.0f;
};

#endif // !__RE_CCOMPAMERA_H__