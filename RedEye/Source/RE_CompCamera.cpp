#include "RE_CompCamera.h"

#include "Application.h"
#include "RE_GameObject.h"
#include "FileSystem.h"
#include "RE_CompTransform.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "OutputLog.h"
#include "ShaderManager.h"

#include "ImGui\imgui.h"
#include "SDL2\include\SDL_opengl.h"

RE_CompCamera::RE_CompCamera(RE_GameObject* go, bool toPerspective, float near_plane, float far_plane)
	: RE_Component(C_CAMERA, go), isPerspective(toPerspective)
{
	transform = (go == nullptr) ? new RE_CompTransform() : go->GetTransform();

	frustum.SetKind(
		math::FrustumProjectiveSpace::FrustumSpaceGL,
		math::FrustumHandedness::FrustumRightHanded);

	if (isPerspective)
		frustum.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	else
		frustum.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());

	//frustum.SetWorldMatrix(math::float3x4::Translate(0.f, 0.f, 0.f));
	frustum.SetViewPlaneDistances(near_plane, far_plane);

	SetVerticalFOV(30.f);

	right = math::vec(1.f, 0.f, 0.f);
	up = math::vec(0.f, 1.f, 0.f);
	front = math::vec(0.f, 0.f, 1.f);

	transform->SetPosition(math::vec(0.f, 5.f, -5.f));

	OnTransformModified();

	RecalculateMatrixes();
}
RE_CompCamera::RE_CompCamera(RE_GameObject * go, bool toPerspective, float near_plane, float far_plane, float h_fov_rads, float v_fov_rads, float h_fov_degrees, float v_fov_degrees, math::vec position, math::vec rotation, math::vec scale)
	: RE_Component(C_CAMERA, go), isPerspective(toPerspective), near_plane(near_plane), far_plane(far_plane), h_fov_rads(h_fov_rads), v_fov_rads(v_fov_rads), h_fov_degrees(h_fov_degrees), v_fov_degrees(v_fov_degrees)
{
	transform = go->GetTransform();
	transform->SetPosition(position);
	transform->SetRotation(rotation);
	transform->SetScale(scale);

	frustum.SetKind(
		math::FrustumProjectiveSpace::FrustumSpaceGL,
		math::FrustumHandedness::FrustumRightHanded);

	if (isPerspective)
		frustum.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	else
		frustum.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());

	//frustum.SetWorldMatrix(math::float3x4::Translate(0.f, 0.f, 0.f));
	frustum.SetViewPlaneDistances(near_plane, far_plane);

	SetVerticalFOV(30.f);

	right = math::vec(1.f, 0.f, 0.f);
	up = math::vec(0.f, 1.f, 0.f);
	front = math::vec(0.f, 0.f, 1.f);

	OnTransformModified();

	RecalculateMatrixes();
}

RE_CompCamera::RE_CompCamera(const RE_CompCamera & cmpCamera, RE_GameObject * go) : RE_Component(C_CAMERA, go)
{
	if (cmpCamera.GetGO() == nullptr && go == nullptr)
		transform = new RE_CompTransform(*cmpCamera.transform);
	else
		transform = (go == nullptr) ? new RE_CompTransform() : go->GetTransform();

	near_plane = cmpCamera.near_plane;
	far_plane = cmpCamera.far_plane;

	h_fov_rads = cmpCamera.h_fov_rads;
	v_fov_rads = cmpCamera.v_fov_rads;

	h_fov_degrees = cmpCamera.h_fov_degrees;
	v_fov_degrees = cmpCamera.v_fov_degrees;

	isPerspective = cmpCamera.isPerspective;

	frustum = cmpCamera.frustum;

	right = cmpCamera.right;
	up = cmpCamera.up;
	front = cmpCamera.front;

	OnTransformModified();

	RecalculateMatrixes();
}


RE_CompCamera::~RE_CompCamera()
{
	if(go == nullptr)
		DEL(transform);
}

void RE_CompCamera::Update()
{
	if (go == nullptr)
	{
		transform->Update();

		if (transform->HasChanged())
		{
			transform->ConfirmChange();
			OnTransformModified();
		}
	}
	
	if (need_recalculation)
		RecalculateMatrixes();
}

void RE_CompCamera::Draw()
{
	if (draw_frustum)
	{
		ShaderManager::use(0);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf((transform->GetMatrixModel() * App->editor->GetCamera()->GetView()).ptr());

		glColor3f(0.f, 255.f, 165.f);
		glLineWidth(3.0f);
		glBegin(GL_LINES);

		for (uint i = 0; i < 12; i++)
		{
			glVertex3f(frustum.Edge(i).a.x, frustum.Edge(i).a.y, frustum.Edge(i).a.z);
			glVertex3f(frustum.Edge(i).b.x, frustum.Edge(i).b.y, frustum.Edge(i).b.z);
		}

		glEnd();
		glLineWidth(1.0f);
	}
}

void RE_CompCamera::DrawProperties()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		float r[3] = { right.x, right.y, right.z };
		ImGui::DragFloat3("Right", r, 0.1f, -10, 10, "%.2f");

		float u[3] = { up.x, up.y, up.z };
		ImGui::DragFloat3("Up", u, 0.1f, -10, 10, "%.2f");

		float f[3] = { front.x, front.y, front.z };
		ImGui::DragFloat3("Front", f, 0.1f, -10, 10, "%.2f");
	}
}

void RE_CompCamera::LocalRotate(float x_angle_rad, float y_angle_rad)
{
	if (x_angle_rad != 0)
	{
		float3x3 rot = float3x3::RotateAxisAngle(vec(0.f, 1.f, 0.f), x_angle_rad);

		right = rot * right;
		up = rot * up;
		front = rot * front;
	}

	if (y_angle_rad != 0)
	{
		float3x3 rot = float3x3::RotateAxisAngle(right, y_angle_rad);

		up = rot * up;
		front = rot * front;

		if (up.y < 0.0f)
		{
			front = vec(0.0f, front.y > 0.0f ? 1.0f : -1.0f, 0.0f);
			up = front.Cross(right);
		}
	}

	right.Normalize();
	up.Normalize();
	front.Normalize();

	OnTransformModified();
}

RE_CompTransform * RE_CompCamera::GetTransform() const
{
	return go != nullptr ? go->GetTransform() : transform;
}

void RE_CompCamera::OnTransformModified()
{
	math::float4x4 global_transform = transform->GetMatrixModel();
	frustum.SetFrame(
		global_transform.Row3(3),
		front,
		up);

	//frustum.SetWorldMatrix(global_transform.Float3x4Part());

	need_recalculation = true;
}

void RE_CompCamera::SetPlanesDistance(float near_plane, float far_plane)
{
	frustum.SetViewPlaneDistances(near_plane, far_plane);
}

void RE_CompCamera::SwapCameraType()
{
	if (isPerspective)
		frustum.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());
	else
		frustum.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	
	isPerspective = !isPerspective;

	need_recalculation = true;
}

float RE_CompCamera::GetVFOVDegrees() const
{
	return v_fov_degrees;
}

float RE_CompCamera::GetHFOVDegrees() const
{
	return h_fov_degrees;
}

void RE_CompCamera::SetVerticalFOV(float vertical_fov_degrees)
{
	if (isPerspective)
	{
		RE_CAPTO(vertical_fov_degrees, 180.0f);

		v_fov_rads = vertical_fov_degrees * DEGTORAD;
		h_fov_rads = 2.0f * math::Atan(math::Tan(v_fov_rads / 2.0f) * App->window->GetAspectRatio());

		h_fov_degrees = h_fov_rads * RADTODEG;
		v_fov_degrees = vertical_fov_degrees;

		frustum.SetPerspective(h_fov_rads, v_fov_rads);

		need_recalculation = true;
	}
}

void RE_CompCamera::ResetAspectRatio()
{
	if (isPerspective)
		frustum.SetVerticalFovAndAspectRatio(v_fov_rads, App->window->GetAspectRatio());
	else
		frustum.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());

	need_recalculation = true;
}

math::float4x4 RE_CompCamera::GetView() const
{
	return calculated_view;
}

float* RE_CompCamera::GetViewPtr() const
{
	return (float*)calculated_view.v;
}

math::float4x4 RE_CompCamera::GetProjection() const
{
	return calculated_projection;
}

float* RE_CompCamera::GetProjectionPtr() const
{
	return (float*)calculated_projection.v;
}

math::Frustum RE_CompCamera::GetFrustum() const
{
	return frustum;
}

void RE_CompCamera::RecalculateMatrixes()
{
	calculated_view = frustum.ViewMatrix();
	calculated_view.Transpose();

	calculated_projection = frustum.ProjectionMatrix();
	calculated_projection.Transpose();

	need_recalculation = false;
}

void RE_CompCamera::LocalMove(Dir dir, float speed)
{
	if (speed != 0.f)
	{
		switch (dir)
		{
		case FORWARD:	transform->SetPosition(transform->GetLocalPosition() + (front * speed)); break;
		case BACKWARD:	transform->SetPosition(transform->GetLocalPosition() - (front * speed)); break;
		case LEFT:		transform->SetPosition(transform->GetLocalPosition() + (right * speed)); break;
		case RIGHT:		transform->SetPosition(transform->GetLocalPosition() - (right * speed)); break;
		case UP:		transform->SetPosition(transform->GetLocalPosition() + (up * speed)); break;
		case DOWN:		transform->SetPosition(transform->GetLocalPosition() - (up * speed)); break;
		}
	}
}

void RE_CompCamera::Orbit(float dx, float dy, RE_GameObject * focus)
{
	focus_global_pos = focus->GetTransform()->GetGlobalPosition();
	math::vec position = transform->GetGlobalPosition();
	float distance = position.Distance(focus_global_pos);

	transform->SetGlobalPosition(focus_global_pos);
	transform->Update();
	LocalRotate(dx, dy);
	transform->SetGlobalPosition(focus_global_pos - (front * distance));
}

void RE_CompCamera::Focus(RE_GameObject * focus, float min_dist)
{
	float camDistance = min_dist;
	math::AABB box = focus->GetGlobalBoundingBox();
	float radius = box.HalfSize().Length();
	focus_global_pos = box.CenterPoint();

	if (radius > 0)
	{
		// Vertical distance
		float v_dist = radius / math::Sin(v_fov_rads / 2.0f);
		if (v_dist > camDistance)
			camDistance = v_dist;

		// Horizontal distance
		float h_dist = radius / math::Sin(h_fov_rads / 2.0f);
		if (h_dist > camDistance)
			camDistance = h_dist;
	}

	transform->SetGlobalPosition(focus_global_pos - (front * camDistance));
	LOG("FocusPos = (%.1f,%.1f,%.1f)", focus_global_pos.x, focus_global_pos.y, focus_global_pos.z);
}

math::vec RE_CompCamera::GetRight() const
{
	return right;
}

math::vec RE_CompCamera::GetUp() const
{
	return up;
}

math::vec RE_CompCamera::GetFront() const
{
	return front;
}

void RE_CompCamera::Serialize(JSONNode * node, rapidjson::Value * comp_array)
{
	rapidjson::Value val(rapidjson::kObjectType);

	val.AddMember(rapidjson::Value::StringRefType("type"), rapidjson::Value().SetInt((int)type), node->GetDocument()->GetAllocator());
	
	val.AddMember(rapidjson::Value::StringRefType("isPrespective"), rapidjson::Value().SetBool(isPerspective), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("near_plane"), rapidjson::Value().SetFloat(near_plane), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("far_plane"), rapidjson::Value().SetFloat(far_plane), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("h_fov_rads"), rapidjson::Value().SetFloat(h_fov_rads), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("v_fov_rads"), rapidjson::Value().SetFloat(v_fov_rads), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("h_fov_degrees"), rapidjson::Value().SetFloat(h_fov_degrees), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("v_fov_degrees"), rapidjson::Value().SetFloat(v_fov_degrees), node->GetDocument()->GetAllocator());


	rapidjson::Value float_array(rapidjson::kArrayType);

	float_array.PushBack(GetTransform()->GetLocalPosition().x, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalPosition().y, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalPosition().z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("position"), float_array.Move(), node->GetDocument()->GetAllocator());

	float_array.SetArray();
	float_array.PushBack(GetTransform()->GetLocalEulerRotation().x, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalEulerRotation().y, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalEulerRotation().z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("rotation"), float_array.Move(), node->GetDocument()->GetAllocator());

	float_array.SetArray();
	float_array.PushBack(GetTransform()->GetLocalScale().x, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalScale().y, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalScale().z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("scale"), float_array.Move(), node->GetDocument()->GetAllocator());

	comp_array->PushBack(val, node->GetDocument()->GetAllocator());
}
