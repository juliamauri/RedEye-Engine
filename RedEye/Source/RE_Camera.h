#ifndef __RE_CAMERA_H__
#define __RE_CAMERA_H__

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "MathGeoLib/include/Math/float4x4.h"

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

#define YAW -90.0f
#define PITCH 0.0f
#define SENSITIVITY 0.1f
#define ZOOM 45.0f


class RE_Camera
{
public:
	//Camera initialize world origin at (0, 0, 0) and planes distance 0.1f near and 100.0f far
	RE_Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

	//Get camera view matrix
	const float* GetView();
	math::float4x4 GetViewMatrixMathGeoLib();

	//Get camera projection matrix
	const float* GetProjection();
	math::float4x4 GetProjectionMatrixMathGeoLib();

	//Move camera
	void Move(Camera_Movement direction, float velocity);

	//Rotate Camera
	void RotateWMouse(float xoffset, float yoffset, bool constrainPitch = true);

	//Zoom
	void ZoomMouse(float yoffset);

private:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MouseSensitivity;
	float Zoom;

	//true = Prespective | false = Orthographic
	bool cameraType;

	void updateCameraVectors();
	math::float4x4 ConvertGLMtoMathGeoLibMatrix(glm::mat4 mat);
};

#endif // !__RE_CAMERA_H__