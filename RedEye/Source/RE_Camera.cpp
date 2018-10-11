#include "RE_Camera.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "TimeManager.h"

RE_Camera::RE_Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

math::float4x4 RE_Camera::GetView()
{
	return ConvertGLMtoMathGeoLibMatrix(glm::lookAt(Position, Front, Up));
}

math::float4x4 RE_Camera::GetProjection()
{
	return ConvertGLMtoMathGeoLibMatrix(glm::perspective(glm::radians(Zoom), (float)(App->window->GetWidth() / App->window->GetHeight()), 0.1f, 100.0f));
}

void RE_Camera::Move(Camera_Movement direction, float velocity)
{
	if (direction == FORWARD)
		Position += Front * velocity;
	if (direction == BACKWARD)
		Position -= Front * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;
}

void RE_Camera::RotateWMouse(float xoffset, float yoffset, bool constrainPitch)
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

void RE_Camera::ZoomMouse(float yoffset)
{
	if (Zoom >= 1.0f && Zoom <= 45.0f)
		Zoom -= yoffset;
	if (Zoom <= 1.0f)
		Zoom = 1.0f;
	if (Zoom >= 45.0f)
		Zoom = 45.0f;
}

void RE_Camera::SetPosition(const math::vec pos)
{
	Position.x = pos.x;
	Position.y = pos.y;
	Position.z = pos.z;

	updateCameraVectors();
}

void RE_Camera::SetFocus(const math::vec f)
{
	glm::vec3 focus = { f.x, f.y, f.z };
	const glm::mat4 inverted = glm::inverse(glm::lookAt(Position, focus, Up));

	Front = -glm::vec3(inverted[2]);
	Yaw = glm::degrees(glm::atan(Front.z, Front.x));
	Pitch = glm::degrees(glm::asin(Front.y));

	Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}

void RE_Camera::updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}

math::float4x4 RE_Camera::ConvertGLMtoMathGeoLibMatrix(glm::mat4 mat)
{
	return math::float4x4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
		mat[1][0], mat[1][1], mat[1][2], mat[1][3],
		mat[2][0], mat[2][1], mat[2][2], mat[2][3],
		mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}