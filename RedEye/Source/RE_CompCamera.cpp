#include "RE_CompCamera.h"

#include "Application.h"
#include "RE_GameObject.h"
#include "FileSystem.h"
#include "RE_CompTransform.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "RE_CameraManager.h"
#include "OutputLog.h"
#include "ShaderManager.h"

#include "ImGui\imgui.h"
#include "SDL2\include\SDL_opengl.h"

RE_CompCamera::RE_CompCamera(RE_GameObject* go, bool toPerspective, float n_plane, float f_plane, float v_fov, short aspect, bool draw_frustum) :
	RE_Component(C_CAMERA, go),
	right(math::vec(1.f, 0.f, 0.f)),
	up(math::vec(0.f, 1.f, 0.f)),
	front(math::vec(0.f, 0.f, 1.f)),
	draw_frustum(draw_frustum)
{
	// Fustrum - Kind
	frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

	// Fustrum - Plane distance
	SetPlanesDistance(n_plane, f_plane);

	// Fustrum - Perspective & Aspect Ratio
	isPerspective = toPerspective;
	v_fov_rads = v_fov;
	SetAspectRatio(AspectRatioTYPE(aspect));

	// Transform
	transform = (go == nullptr) ? new RE_CompTransform() : go->GetTransform();

	OnTransformModified();
	RecalculateMatrixes();
}

RE_CompCamera::RE_CompCamera(const RE_CompCamera & cmpCamera, RE_GameObject * go) :
	RE_Component(C_CAMERA, go),
	right(cmpCamera.right),
	up(cmpCamera.up),
	front(cmpCamera.front),
	draw_frustum(cmpCamera.draw_frustum)
{
	// Fustrum - Kind
	frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

	// Fustrum - Plane distance
	SetPlanesDistance(cmpCamera.near_plane, cmpCamera.far_plane);

	// Fustrum - Perspective & Aspect Ratio
	isPerspective = cmpCamera.isPerspective;
	v_fov_rads = cmpCamera.v_fov_rads;
	SetAspectRatio(cmpCamera.target_ar);

	// Transform
	if (cmpCamera.GetGO() == nullptr && go == nullptr)
		transform = new RE_CompTransform(*cmpCamera.transform);
	else
		transform = (go == nullptr) ? new RE_CompTransform() : go->GetTransform();

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
	else if (transform == nullptr)
		transform = go->GetTransform();
	
	if (need_recalculation)
		RecalculateMatrixes();
}

void RE_CompCamera::DrawProperties()
{
	if (ImGui::CollapsingHeader("Camera"))
		DrawItsProperties();
}

void RE_CompCamera::DrawAsEditorProperties()
{
	if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_(ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)))
	{
		DrawItsProperties();
		ImGui::TreePop();
	}
}

void RE_CompCamera::DrawFrustum() const
{
	if (draw_frustum)
	{
		for (uint i = 0; i < 12; i++)
		{
			glVertex3f(frustum.Edge(i).a.x, frustum.Edge(i).a.y, frustum.Edge(i).a.z);
			glVertex3f(frustum.Edge(i).b.x, frustum.Edge(i).b.y, frustum.Edge(i).b.z);
		}
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
	math::float4x4 trs = transform->GetMatrixModel();
	frustum.SetFrame(
		trs.Row3(3),
		trs.MulDir(front),
		trs.MulDir(up));

	need_recalculation = true;
}

void RE_CompCamera::SetPlanesDistance(float n_plane, float f_plane)
{
	near_plane = n_plane;
	far_plane = f_plane;
	frustum.SetViewPlaneDistances(near_plane, far_plane);
}

void RE_CompCamera::SetFOV(float vertical_fov_degrees)
{
	RE_CAPTO(vertical_fov_degrees, 180.0f);

	v_fov_rads = vertical_fov_degrees * DEGTORAD;
	h_fov_rads = 2.0f * math::Atan(math::Tan(v_fov_rads / 2.0f) * (width / height));

	h_fov_degrees = h_fov_rads * RADTODEG;
	v_fov_degrees = vertical_fov_degrees;

	if (isPerspective)
	{
		frustum.SetPerspective(h_fov_rads, v_fov_rads);
		need_recalculation = true;
	}
}

void RE_CompCamera::SetAspectRatio(AspectRatioTYPE aspect_ratio)
{
	target_ar = aspect_ratio;
	SetBounds(App->window->GetWidth(), App->window->GetHeight());
}

void RE_CompCamera::SetBounds(float w, float h)
{
	switch (target_ar)
	{
	case Fit_Window:
	{
		width = w;
		height = h;
		break;
	}
	case Square_1x1:
	{
		if (w >= h)
			width = height = h;
		else
			width = height = w;

		break;
	}
	case TraditionalTV_4x3:
	{
		if (w / 4.0f >= h / 3.0f)
		{
			width = (h * 4.0f) / 3.0f;
			height = h;
		}
		else
		{
			width = w;
			height = (w * 3.0f) / 4.0f;
		}

		break;
	}
	case Movietone_16x9:
	{
		if (w / 16.0f >= h / 9.0f)
		{
			width = (h * 16.0f) / 9.0f;
			height = h;
		}
		else
		{
			width = w;
			height = (w * 9.0f) / 16.0f;
		}
		break;
	}
	case Personalized:
	{
		width = w;
		height = h;
		break;
	}
	}

	SetFOV(v_fov_rads * RADTODEG);

	if (!isPerspective)
		frustum.SetOrthographic(width, height);

	if (RE_CameraManager::CurrentCamera() == this)
		Event::Push(CURRENT_CAM_VIEWPORT_CHANGED, App->renderer3d);

	need_recalculation = true;
}

float RE_CompCamera::GetTargetWidth() const
{
	return width;
}

float RE_CompCamera::GetTargetHeight() const
{
	return height;
}

void RE_CompCamera::GetTargetWidthHeight(int & w, int & h) const
{
	w = width;
	h = height;
}

void RE_CompCamera::GetTargetWidthHeight(float & w, float & h) const
{
	w = width;
	h = height;
}

void RE_CompCamera::GetTargetViewPort(math::float4& viewPort) const
{
	viewPort.z = height;
	viewPort.w = width;

	int wH = App->window->GetHeight();
	int wW = App->window->GetWidth();

	switch (target_ar)
	{
	case RE_CompCamera::Fit_Window:
		viewPort.x = 0;
		viewPort.y = 0;
		break;
	case RE_CompCamera::Square_1x1:
		viewPort.y = 0;
		viewPort.x = (wW / 2) - (width / 2);
		break;
	case RE_CompCamera::TraditionalTV_4x3:
		viewPort.y = 0;
		viewPort.x = (wW / 2) - (width / 2);
		break;
	case RE_CompCamera::Movietone_16x9:
	{
		viewPort.x = 0;
		if(height < wH) viewPort.y = (wH / 2) - (height / 2);
	}
		break;
	case RE_CompCamera::Personalized:
		viewPort.x = 0;
		viewPort.y = 0;
		break;
	}
}

void RE_CompCamera::SetPerspective()
{
	isPerspective = true;
	frustum.SetVerticalFovAndAspectRatio(v_fov_rads, width / height);
	need_recalculation = true;
}

void RE_CompCamera::SetOrthographic()
{
	isPerspective = false;
	frustum.SetOrthographic(width, height);
	need_recalculation = true;
}

void RE_CompCamera::SwapCameraType()
{
	isPerspective ? SetOrthographic() : SetPerspective();
}

const math::Frustum RE_CompCamera::GetFrustum() const
{
	return frustum;
}

float RE_CompCamera::GetVFOVDegrees() const
{
	return v_fov_degrees;
}

float RE_CompCamera::GetHFOVDegrees() const
{
	return h_fov_degrees;
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

bool RE_CompCamera::OverridesCulling() const
{
	return override_cull;
}

void RE_CompCamera::RecalculateMatrixes()
{
	calculated_view = frustum.ViewMatrix();
	calculated_view.Transpose();

	calculated_projection = frustum.ProjectionMatrix();
	calculated_projection.Transpose();

	need_recalculation = false;
}

void RE_CompCamera::DrawItsProperties()
{
	ImGui::Checkbox("Draw Frustum", &draw_frustum);
	ImGui::Checkbox("Override Culling", &override_cull);

	if (ImGui::DragFloat("Near Plane", &near_plane, 1.0f, 1.0f, far_plane, "%.1f"))
		SetPlanesDistance(near_plane, far_plane);

	if (ImGui::DragFloat("Far Plane", &far_plane, 10.0f, near_plane, 65000.0f, "%.1f"))
		SetPlanesDistance(near_plane, far_plane);

	int aspect_ratio = target_ar;
	if (ImGui::Combo("Aspect Ratio", &aspect_ratio, "Fit Window\0Square 1x1\0TraditionalTV 4x3\0Movietone 16x9\0Personalized\0"))
		SetAspectRatio(AspectRatioTYPE(aspect_ratio));

	if (target_ar == Personalized)
	{
		int w = width;
		int h = height;
		if (ImGui::DragInt("Width", &w, 4.0f, 1, App->window->GetWidth(), "%.1f") ||
			ImGui::DragInt("Height", &h, 4.0f, 1, App->window->GetMaxHeight(), "%.1f"))
			SetBounds(w,h);
	}

	int item_current = isPerspective ? 0 : 1;
	if (ImGui::Combo("Type", &item_current, "Perspective\0Orthographic\0") && (isPerspective != (item_current == 0)))
		SwapCameraType();

	if (isPerspective)
		if (ImGui::DragFloat("FOV", &v_fov_degrees, 1.0f, 0.0f, 180.0f, "%.1f"))
			SetFOV(v_fov_degrees);
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

void RE_CompCamera::Orbit(float dx, float dy, const RE_GameObject& focus)
{
	focus_global_pos = focus.GetGlobalBoundingBox().CenterPoint();
	math::vec position = transform->GetGlobalPosition();
	float distance = position.Distance(focus_global_pos);

	transform->SetGlobalPosition(focus_global_pos);
	transform->Update();
	LocalRotate(dx, dy);
	transform->SetGlobalPosition(focus_global_pos - (front * distance));
}

void RE_CompCamera::Focus(const RE_GameObject* focus, float min_dist)
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

	val.AddMember(rapidjson::Value::StringRefType("v_fov_rads"), rapidjson::Value().SetFloat(v_fov_rads), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("aspect_ratio"), rapidjson::Value().SetInt((int)target_ar), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("draw_frustum"), rapidjson::Value().SetBool(draw_frustum), node->GetDocument()->GetAllocator());

	comp_array->PushBack(val, node->GetDocument()->GetAllocator());
}
