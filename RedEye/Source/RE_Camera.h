#ifndef __RE_CAMERA_H__
#define __RE_CAMERA_H__

#include "RE_Math.h"

enum CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class RE_Camera
{
public:
	//Camera initialize world origin at (0, 0, 0) and planes distance 0.1f near and 100.0f far
	//@param camraType -> true = Prespective | false = Orthographic
	//@param near and @far -> distances of planes from camera position
	RE_Camera(bool cameraType, float near_plane = 0.1f, float far_plane = 100.0f);

	//Set the camera position, must use this
	//@param pos ->camera position
	void SetPos(math::vec pos);

	//Set the camera front vector
	//@param yaw, pitch euler angles
	void SetEulerAngle(float pitch, float yaw);

	//Set the camera front vector
	//@param front -> camera front
	void SetFront(math::vec front);

	//Set World origin Position
	//@param world -> world position
	void SetWorldOrigin(math::vec world);

	//Set the position planes by distance
	//@param near and @far -> distances of planes from camera position
	void SetPlanesDistance(float near_plane, float far_plane);

	//If window size changed, needed to call this function.
	void ResetFov();

	//prespective to orthograohic or inverse
	void ChangeCameraType();

	//Get camera view matrix
	math::float4x4 GetView();

	//Get camera projection matrix
	math::float4x4 GetProjection();

	//LookAt, return the view matrix (view * transformation matrix)
	//@param cameraTarget -> The object that the camera looks
	void LookAt(math::vec cameraTarget);

	float* RealView();

	//Move camera
	void Move(CameraMovement dir, float speed);
	
	void ResetCameraQuat();

private:
	//Camera frustum
	math::Frustum camera;

	math::Quat cameraQuat = math::Quat::identity;

	math::float4x4 view;

	math::vec Up;
	math::vec Front;
	math::vec Right;

	//true = Prespective | false = Orthographic
	bool cameraType;
};

#endif // !__RE_CAMERA_H__