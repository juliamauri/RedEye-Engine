#include "RE_CompCamera.h"

#include "Application.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "ModuleWindow.h"

RE_CompCamera::RE_CompCamera(RE_GameObject* go , bool cameraType, float near_plane, float far_plane) : RE_Component(C_CAMERA,go)
{
	if (go == nullptr)
	{
		transform = new RE_CompTransform();
		transform->setCamera(this);
	}
	else
		transform = go->GetTransform();

	this->cameraType = cameraType;
	camera.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

	if (cameraType)
		camera.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	else
		camera.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());

	camera.SetWorldMatrix(math::float3x4::Translate(math::float3(0.0f, 0.0f, 0.0f)));
	camera.SetViewPlaneDistances(near_plane, far_plane);
}

void RE_CompCamera::PreUpdate()
{
}

void RE_CompCamera::Update()
{
	if (go == nullptr)
		transform->Update();
}

void RE_CompCamera::PostUpdate()
{
}

void RE_CompCamera::Draw()
{
}

void RE_CompCamera::DrawProperties()
{
}


void RE_CompCamera::SetEulerAngle(float p, float y)
{
	yaw += y;
	pitch += p;

	transform->SetRot(math::vec(pitch,yaw,0.0f));



}

void RE_CompCamera::OnTransformModified()
{
	camera.SetWorldMatrix(transform->GetGlobalMatrix().Float3x4Part());
}

void RE_CompCamera::SetPlanesDistance(float near_plane, float far_plane)
{
	camera.SetViewPlaneDistances(near_plane, far_plane);
}

void RE_CompCamera::ResetFov()
{
	if (cameraType)
		camera.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	else
		camera.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());
}

void RE_CompCamera::ChangeCameraType()
{
	if (cameraType)
		camera.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());
	else
		camera.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	
	cameraType = !cameraType;
}

void RE_CompCamera::SetPos(math::vec position)
{
	transform->SetPos(position);
}

math::float4x4 RE_CompCamera::GetView()
{
	math::float4x4 view = transform->GetGlobalMatrix();
	return view;
}

math::float4x4 RE_CompCamera::GetProjection()
{
	return camera.ProjectionMatrix().Transposed();
}

void RE_CompCamera::LookAt(math::vec cameraTarget)
{
	view = GetView() * math::float4x4::LookAt(camera.Front().Normalized(),(camera.Pos() - cameraTarget).Normalized(),camera.Up().Normalized(),math::vec(0.0f,1.0f,0.0f).Normalized());
}

float* RE_CompCamera::RealView()
{
	return view.ptr();
}

void RE_CompCamera::Move(CameraMovement dir, float speed)
{
	switch (dir)
	{
	case FORWARD:
		transform->SetPos(transform->GetPosition() + (speed * transform->GetForward()));
		break;
	case BACKWARD:
		transform->SetPos(transform->GetPosition() - (speed * transform->GetForward()));
		break;
	case LEFT:
		transform->SetPos(transform->GetPosition() + (transform->GetRight() * speed));
		break;
	case RIGHT:
		transform->SetPos(transform->GetPosition() - (transform->GetRight() * speed));
		break;
	}
}