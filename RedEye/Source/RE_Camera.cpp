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

void RE_Camera::SetPos(math::float3 pos)
{
	camera.SetPos(pos);
}

void RE_Camera::SetWorldOrigin(math::float3 world)
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
