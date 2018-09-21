#include "RE_Camera.h"

#include "Application.h"
#include "ModuleWindow.h"

RE_Camera::RE_Camera(bool cameraType, float near_plane, float far_plane)
{
	this->cameraType = cameraType;
	camera.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

	if (cameraType)
		camera.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	else
		camera.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());

	camera.SetWorldMatrix(math::float3x4::Translate(math::float3(0.0f, 0.0f, 0.0f)));
	camera.SetViewPlaneDistances(near_plane, far_plane);
}

void RE_Camera::SetFront(float yaw, float pitch)
{
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	camera.SetFront(math::vec(cos(yaw * DEGTORAD) * cos(pitch * DEGTORAD), sin(pitch * DEGTORAD), sin(yaw * DEGTORAD) * cos(pitch * DEGTORAD)).Normalized());
}

void RE_Camera::SetFront(math::vec front)
{
	camera.SetFront(front);
}

void RE_Camera::SetPos(math::vec pos)
{
	camera.SetPos(pos);
}

void RE_Camera::SetWorldOrigin(math::vec world)
{
	camera.SetWorldMatrix(math::float3x4::Translate(world));
}

void RE_Camera::SetPlanesDistance(float near_plane, float far_plane)
{
	camera.SetViewPlaneDistances(near_plane, far_plane);
}

void RE_Camera::ResetFov()
{
	if (cameraType)
		camera.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	else
		camera.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());
}

void RE_Camera::ChangeCameraType()
{
	if (cameraType)
		camera.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());
	else
		camera.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	
	cameraType = !cameraType;
}

math::float4x4 RE_Camera::GetView()
{
	math::float4x4 view = camera.ViewMatrix();
	view.InverseTranspose();
	return view;
}

math::float4x4 RE_Camera::GetProjection()
{
	return camera.ProjectionMatrix().Transposed();
}

math::float4x4 RE_Camera::LookAt(math::vec cameraTarget)
{
	return GetView() * math::float4x4::LookAt(camera.Front().Normalized(),(camera.Pos() - cameraTarget).Normalized(),camera.Up().Normalized(),math::vec(0.0f,1.0f,0.0f).Normalized());
}

void RE_Camera::MoveFront(float speed)
{
	camera.SetPos(camera.Pos() - (speed * camera.Front()));
}

void RE_Camera::MoveBack(float speed)
{
	camera.SetPos(camera.Pos() + (speed * camera.Front()));
}

void RE_Camera::MoveLeft(float speed)
{
	camera.SetPos(camera.Pos() + (camera.Front().Cross(camera.Up()).Normalized() * speed));
}

void RE_Camera::MoveRight(float speed)
{
	camera.SetPos(camera.Pos() - (camera.Front().Cross(camera.Up()).Normalized() * speed));
}
