#include "RE_CompCamera.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_ShaderImporter.h"
#include "ModuleEditor.h"
#include "RE_InternalResources.h"
#include "RE_ResourceManager.h"

#include "RE_SkyBox.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"

#include "RE_CameraManager.h"
#include "OutputLog.h"

#include "ImGui\imgui.h"
#include "SDL2\include\SDL_opengl.h"

RE_CompCamera::RE_CompCamera() :RE_Component(C_CAMERA, nullptr){
}

RE_CompCamera::~RE_CompCamera()
{
	if(go == nullptr)
		DEL(transform);
}

void RE_CompCamera::SetUp(RE_GameObject* parent, bool toPerspective, float n_plane, float f_plane, float v_fov, short aspect, bool draw_frustum, bool usingSkybox, const char* skyboxMD5)
{
	go = parent;
	if(parent) parent->AddComponent(this);
	right = math::vec(1.f, 0.f, 0.f);
	up = math::vec(0.f, 1.f, 0.f);
	front = math::vec(0.f, 0.f, 1.f);
	this->draw_frustum = draw_frustum;
	this->usingSkybox = usingSkybox;
	this->skyboxMD5 = skyboxMD5;

	// Fustrum - Kind
	frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

	// Fustrum - Plane distance
	SetPlanesDistance(n_plane, f_plane);

	// Fustrum - Perspective & Aspect Ratio
	isPerspective = toPerspective;
	v_fov_rads = v_fov;
	SetAspectRatio(AspectRatioTYPE(aspect));

	// Transform
	if (go == nullptr) {
		transform = new RE_CompTransform();
		transform->SetUp(nullptr);
	}
	else {
		transform = go->GetTransform();
	}

	OnTransformModified();
	RecalculateMatrixes();	// Fustrum - Kind
	frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

	// Fustrum - Plane distance
	SetPlanesDistance(n_plane, f_plane);

	// Fustrum - Perspective & Aspect Ratio
	isPerspective = toPerspective;
	v_fov_rads = v_fov;
	SetAspectRatio(AspectRatioTYPE(aspect));

	// Transform
	if (go == nullptr) {
		transform = new RE_CompTransform();
		transform->SetUp(nullptr);
	}
	else {
		transform = go->GetTransform();
	}

	OnTransformModified();
	RecalculateMatrixes();
}

void RE_CompCamera::SetUp(const RE_CompCamera& cmpCamera, RE_GameObject* parent)
{
	go = parent;
	parent->AddComponent(this);
	right = cmpCamera.right;
	up = cmpCamera.up;
	front = cmpCamera.front;
	draw_frustum = cmpCamera.draw_frustum;
	usingSkybox = cmpCamera.usingSkybox;
	skyboxMD5 = cmpCamera.skyboxMD5;

	// Fustrum - Kind
	frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

	// Fustrum - Plane distance
	SetPlanesDistance(cmpCamera.near_plane, cmpCamera.far_plane);

	// Fustrum - Perspective & Aspect Ratio
	isPerspective = cmpCamera.isPerspective;
	v_fov_rads = cmpCamera.v_fov_rads;
	SetAspectRatio(cmpCamera.target_ar);

	// Transform
	if (go == nullptr) {
		transform = new RE_CompTransform();
		transform->SetUp(nullptr);
	}
	else {
		transform = go->GetTransform();
	}

	OnTransformModified();
	RecalculateMatrixes();
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
	if (ImGui::CollapsingHeader("Camera")) {
		DrawItsProperties();

		ImGui::Separator();
		
		ImGui::Checkbox("Use skybox", &usingSkybox);

		if (usingSkybox) {
			RE_SkyBox* skyRes = (skyboxMD5) ? (RE_SkyBox*)App->resources->At(skyboxMD5) : (RE_SkyBox*)App->resources->At(App->internalResources->GetDefaultSkyBox());

			if (!skyboxMD5) ImGui::Text("This component camera is using the default skybox.");

			if (ImGui::Button(skyRes->GetName()))
				App->resources->PushSelected(skyRes->GetMD5(), true);

			if (skyboxMD5) {
				ImGui::SameLine();
				if (ImGui::Button("Back to Default Skybox"))
					skyboxMD5 = nullptr;
			}

			if (ImGui::BeginMenu("Change skybox"))
			{
				eastl::vector<ResourceContainer*> materials = App->resources->GetResourcesByType(Resource_Type::R_SKYBOX);
				bool none = true;
				for (auto material : materials) {
					if (material->isInternal())
						continue;
					none = false;
					if (ImGui::MenuItem(material->GetName())) {
						if (skyboxMD5) App->resources->UnUse(skyboxMD5);
						skyboxMD5 = material->GetMD5();
						if (skyboxMD5) App->resources->Use(skyboxMD5);
					}
				}
				if (none) ImGui::Text("No custom skyboxes on assets");
				ImGui::EndMenu();
			}
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#SkyboxReference")) {
					if (skyboxMD5) App->resources->UnUse(skyboxMD5);
					skyboxMD5 = *static_cast<const char**>(dropped->Data);
					if (skyboxMD5) App->resources->Use(skyboxMD5);
				}
				ImGui::EndDragDropTarget();
			}
		}
	}
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

	need_recalculation = true;
}

void RE_CompCamera::ForceFOV(float vertical_fov_degrees, float horizontal_fov_degrees)
{
	RE_CAPTO(vertical_fov_degrees, 180.0f);
	RE_CAPTO(horizontal_fov_degrees, 180.0f);

	v_fov_rads = vertical_fov_degrees * DEGTORAD;
	h_fov_rads = horizontal_fov_degrees * DEGTORAD;

	h_fov_degrees = horizontal_fov_degrees;
	v_fov_degrees = vertical_fov_degrees;

	if (isPerspective)
	{
		frustum.SetPerspective(h_fov_rads, v_fov_rads);
		need_recalculation = true;
	}
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
	viewPort.z = width;
	viewPort.w = height;

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
	if (ImGui::Combo("Aspect Ratio", &aspect_ratio, "Fit Window\0Square 1x1\0TraditionalTV 4x3\0Movietone 16x9\0Personalized\0")) {
		target_ar = AspectRatioTYPE(aspect_ratio);
		Event::Push(RE_EventType::UPDATE_SCENE_WINDOWS, App->editor, Cvar(GetGO()));
	}

	//if (target_ar == Personalized)
	//{
	//	int w = width;
	//	int h = height;
	//	if (ImGui::DragInt("Width", &w, 4.0f, 1, App->window->GetWidth(), "%.1f") ||
	//		ImGui::DragInt("Height", &h, 4.0f, 1, App->window->GetMaxHeight(), "%.1f"))
	//		SetBounds(w,h);
	//}

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
	math::AABB box = focus->GetGlobalBoundingBoxWithChilds();
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

void RE_CompCamera::Focus(math::vec center, float radius, float min_dist)
{
	float camDistance = min_dist;
	focus_global_pos = center;

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

eastl::vector<const char*> RE_CompCamera::GetAllResources()
{
	eastl::vector<const char*> ret;

	if (skyboxMD5) ret.push_back(skyboxMD5);

	return ret;
}

void RE_CompCamera::SerializeJson(JSONNode * node, eastl::map<const char*, int>* resources)
{
	node->PushBool("isPrespective", isPerspective);
	node->PushFloat("near_plane", near_plane);
	node->PushFloat("far_plane", far_plane);
	node->PushFloat("v_fov_rads", v_fov_rads);
	node->PushInt("aspect_ratio", target_ar);
	node->PushBool("draw_frustum", draw_frustum);
	node->PushBool("usingSkybox", usingSkybox);
	node->PushInt("skyboxResource", (skyboxMD5) ? resources->at(skyboxMD5) : -1);
}

void RE_CompCamera::DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources, RE_GameObject* parent)
{
	usingSkybox = node->PullBool("usingSkybox", true);
	int sbRes = node->PullInt("skyboxResource", -1);
	SetUp(parent, node->PullBool("isPrespective", true),
		node->PullFloat("near_plane", 1.0f), node->PullFloat("far_plane", 5000.0f),
		node->PullFloat("v_fov_rads", 0.523599f), node->PullInt("aspect_ratio", 0),
		node->PullBool("draw_frustum", true), usingSkybox,
		(sbRes != -1) ? resources->at(sbRes) : nullptr);
}

unsigned int RE_CompCamera::GetBinarySize() const
{
	return sizeof(bool) * 3 + sizeof(int) * 2 + sizeof(float) * 3;
}


void RE_CompCamera::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	size_t size = sizeof(bool);
	memcpy(cursor, &isPerspective, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &near_plane, size);
	cursor += size;

	memcpy(cursor, &far_plane, size);
	cursor += size;

	memcpy(cursor, &v_fov_rads, size);
	cursor += size;

	size = sizeof(int);
	int aspectRatioInt = (int)target_ar;
	memcpy(cursor, &aspectRatioInt, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(cursor, &draw_frustum, size);
	cursor += size;

	memcpy(cursor, &usingSkybox, size);
	cursor += size;

	size = sizeof(int);
	int sbres = (skyboxMD5) ? resources->at(skyboxMD5) : -1;
	memcpy(cursor, &sbres, size);
	cursor += size;
}

void RE_CompCamera::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources, RE_GameObject* parent)
{
	size_t size = sizeof(bool);
	memcpy(&isPerspective, cursor, size);
	cursor += size;

	size = sizeof(float);
	memcpy(&near_plane, cursor, size);
	cursor += size;

	memcpy(&far_plane, cursor, size);
	cursor += size;

	memcpy(&v_fov_rads, cursor,  size);
	cursor += size;

	size = sizeof(int);
	int aspectRatioInt;
	memcpy(&aspectRatioInt, cursor, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(&draw_frustum, cursor, size);
	cursor += size;

	memcpy(&usingSkybox, cursor, size);
	cursor += size;

	size = sizeof(int);
	int sbRes = -1;
	memcpy(&sbRes, cursor,  size);
	cursor += size;

	SetUp(parent, isPerspective, near_plane, far_plane, v_fov_rads, aspectRatioInt, draw_frustum, usingSkybox, (sbRes != -1) ? resources->at(sbRes) : nullptr);
}

bool RE_CompCamera::isUsingSkybox() const
{
	return usingSkybox;
}

const char* RE_CompCamera::GetSkybox() const
{
	return skyboxMD5;
}

void RE_CompCamera::DrawSkybox() const
{
	RE_SkyBox* skyRes = (skyboxMD5) ? (RE_SkyBox*)App->resources->At(skyboxMD5) : (RE_SkyBox*)App->resources->At(App->internalResources->GetDefaultSkyBox());
	skyRes->DrawSkybox();
}

void RE_CompCamera::DeleteSkybox()
{
	usingSkybox = false;
	skyboxMD5 = nullptr;
}

void RE_CompCamera::UseResources()
{
	if (skyboxMD5) App->resources->Use(skyboxMD5);
}

void RE_CompCamera::UnUseResources()
{
	if (skyboxMD5) App->resources->UnUse(skyboxMD5);
}
