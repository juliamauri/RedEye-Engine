#include "RE_CompCamera.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "RE_FileSystem.h"
#include "RE_ShaderImporter.h"
#include "RE_InternalResources.h"
#include "RE_ResourceManager.h"
#include "RE_CameraManager.h"

#include "RE_SkyBox.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_GOManager.h"
#include "OutputLog.h"

#include "ImGui\imgui.h"
#include "SDL2\include\SDL_opengl.h"

RE_CompCamera::RE_CompCamera() : RE_Component(C_CAMERA) {}

RE_CompCamera::~RE_CompCamera()
{
	if(!useParent && transform != nullptr) DEL(transform);
}

void RE_CompCamera::SetProperties(bool toPerspective, float n_plane, float f_plane, float v_fov, short aspect, bool _draw_frustum, bool _usingSkybox, const char* _skyboxMD5)
{
	draw_frustum = _draw_frustum;
	usingSkybox = _usingSkybox;
	skyboxMD5 = _skyboxMD5;

	// Fustrum - Kind
	frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

	// Fustrum - Plane distance
	SetPlanesDistance(n_plane, f_plane);

	// Fustrum - Perspective & Aspect Ratio
	isPerspective = toPerspective;
	v_fov_rads = v_fov;
	SetAspectRatio(AspectRatioTYPE(aspect));

	// Transform
	if (!useParent) (transform = new RE_CompTransform())->PoolSetUp(nullptr, 0ull);

	OnTransformModified();
	RecalculateMatrixes();
}

void RE_CompCamera::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent)
{
	pool_gos = pool;
	if (useParent = (go = parent)) pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompCamera* cmpCamera = dynamic_cast<RE_CompCamera*>(copy);
	SetProperties(cmpCamera->isPerspective, cmpCamera->frustum.NearPlaneDistance(), cmpCamera->frustum.FarPlaneDistance(), cmpCamera->v_fov_rads, cmpCamera->target_ar, cmpCamera->draw_frustum, cmpCamera->usingSkybox, cmpCamera->skyboxMD5);
}

void RE_CompCamera::Update()
{
	if (!useParent && transform->CheckUpdate()) need_recalculation = true;
	if (need_recalculation) RecalculateMatrixes();
}

void RE_CompCamera::OnTransformModified() { need_recalculation = true; }

void RE_CompCamera::DrawProperties()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		DrawItsProperties();
		ImGui::Separator();
		ImGui::Checkbox("Use skybox", &usingSkybox);

		if (usingSkybox)
		{
			RE_SkyBox* skyRes = nullptr;

			if (skyboxMD5)
			{
				skyRes = dynamic_cast<RE_SkyBox*>(App::resources->At(skyboxMD5));
				if (ImGui::Button(skyRes->GetName())) App::resources->PushSelected(skyRes->GetMD5(), true);
				ImGui::SameLine();
				if (ImGui::Button("Back to Default Skybox")) skyboxMD5 = nullptr;
			}
			else
			{
				ImGui::Text("This component camera is using the default skybox.");
				skyRes = dynamic_cast<RE_SkyBox*>(App::resources->At(App::internalResources.GetDefaultSkyBox()));
				if (ImGui::Button(skyRes->GetName())) App::resources->PushSelected(skyRes->GetMD5(), true);
			}

			if (ImGui::BeginMenu("Change skybox"))
			{
				eastl::vector<ResourceContainer*> materials = App::resources->GetResourcesByType(Resource_Type::R_SKYBOX);
				bool none = true;
				for (auto material : materials)
				{
					if (material->isInternal()) continue;

					none = false;
					if (ImGui::MenuItem(material->GetName()))
					{
						if (skyboxMD5) App::resources->UnUse(skyboxMD5);
						skyboxMD5 = material->GetMD5();
						if (skyboxMD5) App::resources->Use(skyboxMD5);
					}
				}
				if (none) ImGui::Text("No custom skyboxes on assets");
				ImGui::EndMenu();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#SkyboxReference"))
				{
					if (skyboxMD5) App::resources->UnUse(skyboxMD5);
					skyboxMD5 = *static_cast<const char**>(dropped->Data);
					if (skyboxMD5) App::resources->Use(skyboxMD5);
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

void RE_CompCamera::DrawSkybox() const
{
	RE_SkyBox* skyRes = dynamic_cast<RE_SkyBox*>(App::resources->At(skyboxMD5 ? skyboxMD5 : App::internalResources.GetDefaultSkyBox()));
	if (skyRes) skyRes->DrawSkybox();
}

void RE_CompCamera::SetPlanesDistance(float n_plane, float f_plane)
{
	frustum.SetViewPlaneDistances(n_plane, f_plane);
	need_recalculation = true;
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

void RE_CompCamera::SetAspectRatio(AspectRatioTYPE aspect_ratio)
{
	target_ar = aspect_ratio;
	SetBounds(static_cast<float>(App::window->GetWidth()), static_cast<float>(App::window->GetHeight()));
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
		if (w >= h) width = height = h;
		else width = height = w;
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
	if (!isPerspective) frustum.SetOrthographic(width, height);
	need_recalculation = true;
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

void RE_CompCamera::SwapCameraType() { isPerspective ? SetOrthographic() : SetPerspective(); }

float RE_CompCamera::GetTargetWidth() const { return width; }
float RE_CompCamera::GetTargetHeight() const { return height; }

void RE_CompCamera::GetTargetWidthHeight(int & w, int & h) const
{
	w = static_cast<int>(width);
	h = static_cast<int>(height);
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

	float wH = static_cast<float>(App::window->GetHeight());
	float wW = static_cast<float>(App::window->GetWidth());

	switch (target_ar)
	{
	case RE_CompCamera::Fit_Window:
	{
		viewPort.x = 0.f;
		viewPort.y = 0.f;
		break;
	}
	case RE_CompCamera::Square_1x1:
	{
		viewPort.y = 0.f;
		viewPort.x = (wW / 2.f) - (width / 2.f);
		break; 
	}
	case RE_CompCamera::TraditionalTV_4x3:
	{
		viewPort.y = 0.f;
		viewPort.x = (wW / 2.f) - (width / 2.f);
		break; 
	}
	case RE_CompCamera::Movietone_16x9:
	{
		viewPort.x = 0.f;
		if (height < wH) viewPort.y = (wH / 2.f) - (height / 2.f);
		break; 
	}
	case RE_CompCamera::Personalized:
	{
		viewPort.x = 0.f;
		viewPort.y = 0.f;
		break; 
	}
	}
}

bool RE_CompCamera::isPrespective() const { return isPerspective; }
float RE_CompCamera::GetNearPlane() const { return frustum.NearPlaneDistance(); }
float RE_CompCamera::GetFarPlane() const { return frustum.FarPlaneDistance(); }
bool RE_CompCamera::OverridesCulling() const { return override_cull; }
const math::Frustum RE_CompCamera::GetFrustum() const { return frustum; }
float RE_CompCamera::GetVFOVDegrees() const { return v_fov_degrees; }
float RE_CompCamera::GetHFOVDegrees() const { return h_fov_degrees; }

RE_CompTransform* RE_CompCamera::GetTransform() const
{
	return useParent ? GetGOCPtr()->GetTransformPtr() : transform;
}

math::float4x4 RE_CompCamera::GetView()
{
	if (need_recalculation) RecalculateMatrixes();
	return global_view;
}

const float* RE_CompCamera::GetViewPtr()
{
	if (need_recalculation) RecalculateMatrixes();
	return global_view.ptr();
}

math::float4x4 RE_CompCamera::GetProjection()
{
	if (need_recalculation) RecalculateMatrixes();
	return global_projection;
}

const float* RE_CompCamera::GetProjectionPtr()
{
	if (need_recalculation) RecalculateMatrixes();
	return global_projection.ptr();
}

void RE_CompCamera::LocalPan(float x_angle_rad, float y_angle_rad)
{
	if (x_angle_rad != 0.f || y_angle_rad != 0.f)
	{
		GetTransform()->LocalPan(x_angle_rad, y_angle_rad);
		need_recalculation = true;
	}
}

void RE_CompCamera::LocalMove(Dir dir, float speed)
{
	if (speed != 0.f)
	{
		GetTransform()->LocalMove(dir, speed);
		need_recalculation = true;
	}
}

void RE_CompCamera::Orbit(float dx, float dy, math::vec center)
{
	if (dx != 0.f || dy != 0.f)
	{
		GetTransform()->Orbit(dx, dy, center);
		need_recalculation = true;
	}
}

void RE_CompCamera::Focus(math::vec center, float radius, float min_dist)
{
	GetTransform()->Focus(center, v_fov_rads, h_fov_rads, radius, min_dist);
	need_recalculation = true;
}

bool RE_CompCamera::isUsingSkybox() const { return usingSkybox; }
const char* RE_CompCamera::GetSkybox() const { return skyboxMD5; }
void RE_CompCamera::DeleteSkybox() { usingSkybox = false; skyboxMD5 = nullptr; }
void RE_CompCamera::SetSkyBox(const char* resS) { skyboxMD5 = resS; }
void RE_CompCamera::UseResources() { if (skyboxMD5) App::resources->Use(skyboxMD5); }
void RE_CompCamera::UnUseResources() { if (skyboxMD5) App::resources->UnUse(skyboxMD5); }

eastl::vector<const char*> RE_CompCamera::GetAllResources()
{
	eastl::vector<const char*> ret;
	if (skyboxMD5) ret.push_back(skyboxMD5);
	return ret;
}

unsigned int RE_CompCamera::GetBinarySize() const
{
	return sizeof(bool) * 3 + sizeof(int) * 2 + sizeof(float) * 3;
}

void RE_CompCamera::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(bool);
	memcpy(cursor, &isPerspective, size);
	cursor += size;

	float near_plane = frustum.NearPlaneDistance();
	float far_plane = frustum.FarPlaneDistance();

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

void RE_CompCamera::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(bool);
	memcpy(&isPerspective, cursor, size);
	cursor += size;

	float near_plane = frustum.NearPlaneDistance();
	float far_plane = frustum.FarPlaneDistance();

	size = sizeof(float);
	memcpy(&near_plane, cursor, size);
	cursor += size;

	memcpy(&far_plane, cursor, size);
	cursor += size;

	memcpy(&v_fov_rads, cursor, size);
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
	memcpy(&sbRes, cursor, size);
	cursor += size;

	SetProperties(isPerspective, near_plane, far_plane, v_fov_rads, aspectRatioInt, draw_frustum, usingSkybox, (sbRes != -1) ? resources->at(sbRes) : nullptr);
}

void RE_CompCamera::SerializeJson(JSONNode * node, eastl::map<const char*, int>* resources) const
{
	node->PushBool("isPrespective", isPerspective);
	node->PushFloat("near_plane", frustum.NearPlaneDistance());
	node->PushFloat("far_plane", frustum.FarPlaneDistance());
	node->PushFloat("v_fov_rads", v_fov_rads);
	node->PushInt("aspect_ratio", target_ar);
	node->PushBool("draw_frustum", draw_frustum);
	node->PushBool("usingSkybox", usingSkybox);
	node->PushInt("skyboxResource", (skyboxMD5) ? resources->at(skyboxMD5) : -1);
}

void RE_CompCamera::DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources)
{
	usingSkybox = node->PullBool("usingSkybox", true);
	int sbRes = node->PullInt("skyboxResource", -1);

	SetProperties(node->PullBool("isPrespective", true),
		node->PullFloat("near_plane", 1.0f), node->PullFloat("far_plane", 5000.0f),
		node->PullFloat("v_fov_rads", 0.523599f), node->PullInt("aspect_ratio", 0),
		node->PullBool("draw_frustum", true), usingSkybox,
		(sbRes != -1) ? resources->at(sbRes) : nullptr);
}

void RE_CompCamera::RecalculateMatrixes()
{
	math::float4x4 trs = GetTransform()->GetGlobalMatrix();

	math::vec front = -trs.Row3(2).Normalized();
	math::vec up = trs.Row3(1).Normalized();
	//math::vec up = front.Cross(math::vec(1.f, 0.f, 0.f)).Normalized();

	frustum.SetFrame(trs.Row3(3), front, up); //trs.Row3(1).Normalized());
	//frustum.SetWorldMatrix(trs.Transposed().Float3x4Part());

	RE_LOG("CAMERA SetFrame(%s, front %s, up %s)", trs.Row3(3).ToString().c_str(), front.ToString().c_str(), up.ToString().c_str());

	global_view = frustum.ViewMatrix();
	global_view.Transpose();
	global_projection = frustum.ProjectionMatrix();
	global_projection.Transpose();

	need_recalculation = false;
}

void RE_CompCamera::DrawItsProperties()
{
	ImGui::Checkbox("Draw Frustum", &draw_frustum);
	ImGui::Checkbox("Override Culling", &override_cull);

	float near_plane = frustum.NearPlaneDistance();
	float far_plane = frustum.FarPlaneDistance();
	if (ImGui::DragFloat("Near Plane", &near_plane, 1.0f, 1.0f, far_plane, "%.1f")) SetPlanesDistance(near_plane, far_plane);
	if (ImGui::DragFloat("Far Plane", &far_plane, 10.0f, near_plane, 65000.0f, "%.1f")) SetPlanesDistance(near_plane, far_plane);

	int aspect_ratio = target_ar;
	if (ImGui::Combo("Aspect Ratio", &aspect_ratio, "Fit Window\0Square 1x1\0TraditionalTV 4x3\0Movietone 16x9\0Personalized\0"))
	{
		target_ar = AspectRatioTYPE(aspect_ratio);
		Event::Push(RE_EventType::UPDATE_SCENE_WINDOWS, App::editor, go);
	}

	// TODO Rub: Personalied Aspect Ratio
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

	if (isPerspective && ImGui::DragFloat("FOV", &v_fov_degrees, 1.0f, 0.0f, 180.0f, "%.1f"))
		SetFOV(v_fov_degrees);
}
