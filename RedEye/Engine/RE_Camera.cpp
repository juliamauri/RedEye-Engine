#include "RE_Camera.h"



#include "RE_ConsoleLog.h"






#include "RE_Math.h"
#include "RE_Memory.h"
#include "Application.h"
#include "RE_ResourceManager.h"

#include "RE_InternalResources.h"
#include "RE_SkyBox.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>
#include <SDL2/SDL_opengl.h>

RE_Camera& RE_Camera::operator=(const RE_Camera& other)
{
	frustum = other.frustum;
	ar = other.ar;
	bounds = other.bounds;
	usingSkybox = other.usingSkybox;
	skyboxMD5 = other.skyboxMD5;
	return *this;
}

void RE_Camera::SetupFrustum(math::float2 _bounds, AspectRatio _ar, float v_fov, float n_plane, float f_plane)
{
	ar = _ar;
	SetBounds(_bounds);

	frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);
	frustum.SetViewPlaneDistances(n_plane, f_plane);
	frustum.SetFrame(math::vec::zero, math::vec::unitZ, math::vec::unitY);

	if (v_fov > 0.f) frustum.SetVerticalFovAndAspectRatio(v_fov, bounds.x / bounds.y);
	else frustum.SetOrthographic(bounds.x, bounds.y);
}

void RE_Camera::SetBounds(math::float2 _bounds)
{
	ARContainBounds(ar, _bounds);
	bounds = _bounds;

	if (frustum.Type() == math::FrustumType::OrthographicFrustum)
		frustum.SetOrthographic(bounds.x, bounds.y);
}

void RE_Camera::SetFrame(const math::vec& pos, const math::vec& front, const math::vec& up)
{
	frustum.SetFrame(pos, front, up);
}

math::LineSegment RE_Camera::UnProjectLineSegment(math::float2 coordinates)
{
	return frustum.UnProjectLineSegment(
			(coordinates.x - (bounds.x / 2.0f)) / (bounds.x / 2.0f),
			((bounds.y - coordinates.y) - (bounds.y / 2.0f)) / (bounds.y / 2.0f));
}

void RE_Camera::ARContainBounds(AspectRatio ar, math::float2& bounds)
{
	switch (ar)
	{
	case AspectRatio::Square_1x1:

		if (bounds.x >= bounds.y) bounds.x = bounds.y;
		else bounds.y = bounds.x;
		break;

	case AspectRatio::TraditionalTV_4x3:

		if (bounds.x / 4.0f >= bounds.y / 3.0f) bounds.x = (bounds.y * 4.0f) / 3.0f;
		else bounds.y = (bounds.x * 3.0f) / 4.0f;
		break;

	case AspectRatio::Movietone_16x9:

		if (bounds.x / 16.0f >= bounds.y / 9.0f) bounds.x = (bounds.y * 16.0f) / 9.0f;
		else bounds.y = (bounds.x * 9.0f) / 16.0f;
		break;

	default: break;
	}
}

#pragma region Draws

void RE_Camera::DrawProperties()
{
	float near_plane = frustum.NearPlaneDistance();
	float far_plane = frustum.FarPlaneDistance();
	if (ImGui::DragFloat("Near Plane", &near_plane, 1.0f, 1.0f, far_plane - 0.1f, "%.1f")) SetPlanesDistance(near_plane, far_plane);
	if (ImGui::DragFloat("Far Plane", &far_plane, 10.0f, near_plane + 0.1f, 65000.0f, "%.1f")) SetPlanesDistance(near_plane, far_plane);

	auto aspect_ratio = static_cast<int>(ar);
	if (ImGui::Combo("Aspect Ratio", &aspect_ratio, "Fit Window\0Square 1x1\0TraditionalTV 4x3\0Movietone 16x9\0Personalized\0"))
		SetAspectRatio(static_cast<AspectRatio>(aspect_ratio));

	// TODO Rub: Custom Aspect Ratio
	//if (ar == Custom)
	//{
	//	int w = width;
	//	int h = height;
	//	if (ImGui::DragInt("Width", &w, 4.0f, 1, App->window->GetWidth(), "%.1f") ||
	//		ImGui::DragInt("Height", &h, 4.0f, 1, App->window->GetMaxHeight(), "%.1f"))
	//		SetBounds(w,h);
	//}

	int current_type = IsPerspective();
	if (ImGui::Combo("Type", &current_type, "Orthographic\0Perspective\0"))
		SwapCameraType();


	auto v_fov_degrees = frustum.VerticalFov() * RE_Math::rad_to_deg;
	if (IsPerspective() && ImGui::DragFloat("FOV", &v_fov_degrees, 1.0f, 0.0f, 180.0f, "%.1f"))
		SetFOVDegrees(v_fov_degrees);

	// TODO: Remove debug functionality of showing all camera matrixes
	ImGui::Separator();
	ImGui::Text("v_fov: %f\nn_plane: %f\nf_plane: %f\nView: %s\nProjection: %s\nPosition: %s",
		frustum.VerticalFov() * RE_Math::rad_to_deg,
		frustum.NearPlaneDistance(),
		frustum.FarPlaneDistance(),
		frustum.ViewMatrix().ToString().c_str(),
		frustum.ProjectionMatrix().ToString().c_str(),
		frustum.Pos().ToString().c_str());

	ImGui::Separator();
	ImGui::Checkbox("Use skybox", &usingSkybox);

	if (usingSkybox)
	{
		RE_SkyBox* skyRes = nullptr;

		if (skyboxMD5)
		{
			skyRes = dynamic_cast<RE_SkyBox*>(RE_RES->At(skyboxMD5));
			if (ImGui::Button(skyRes->GetName())) RE_RES->PushSelected(skyRes->GetMD5(), true);
			ImGui::SameLine();
			if (ImGui::Button("Back to Default Skybox")) skyboxMD5 = nullptr;
		}
		else
		{
			ImGui::Text("This camera is using the default skybox.");
			skyRes = dynamic_cast<RE_SkyBox*>(RE_RES->At(RE_InternalResources::GetDefaultSkyBox()));
			if (ImGui::Button(skyRes->GetName())) RE_RES->PushSelected(skyRes->GetMD5(), true);
		}

		if (ImGui::BeginMenu("Change skybox"))
		{
			eastl::vector<ResourceContainer*> materials = RE_RES->GetResourcesByType(ResourceContainer::Type::SKYBOX);
			bool none = true;
			for (auto material : materials)
			{
				if (material->isInternal()) continue;

				none = false;
				if (ImGui::MenuItem(material->GetName()))
				{
					if (skyboxMD5) RE_RES->UnUse(skyboxMD5);
					skyboxMD5 = material->GetMD5();
					if (skyboxMD5) RE_RES->Use(skyboxMD5);
				}
			}
			if (none) ImGui::Text("No custom skyboxes on assets");
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#SkyboxReference"))
			{
				if (skyboxMD5) RE_RES->UnUse(skyboxMD5);
				skyboxMD5 = *static_cast<const char**>(dropped->Data);
				if (skyboxMD5) RE_RES->Use(skyboxMD5);
			}
			ImGui::EndDragDropTarget();
		}
	}
}

void RE_Camera::DrawFrustum() const
{
	for (uint i = 0; i < 12; i++)
	{
		glVertex3f(frustum.Edge(i).a.x, frustum.Edge(i).a.y, frustum.Edge(i).a.z);
		glVertex3f(frustum.Edge(i).b.x, frustum.Edge(i).b.y, frustum.Edge(i).b.z);
	}
}

#pragma endregion

#pragma region Camera Controls

void RE_Camera::Move(Direction dir, float speed)
{
	if (speed == 0.f) return;

	math::vec pos = frustum.Pos();
	switch (dir)
	{
	case Direction::FORWARD:  pos += frustum.Front() * speed; break;
	case Direction::BACKWARD: pos -= frustum.Front() * speed; break;
	case Direction::LEFT:	  pos -= frustum.Front().Cross(frustum.Up()) * speed; break;
	case Direction::RIGHT:	  pos += frustum.Front().Cross(frustum.Up()) * speed; break;
	case Direction::UP:		  pos += frustum.Up() * speed; break;
	case Direction::DOWN:	  pos -= frustum.Up() * speed; break;
	default: break;
	}

	frustum.SetPos(pos);
}

void RE_Camera::Pan(float rad_dx, float rad_dy, float rad_dz)
{
	if (rad_dx == 0.f && rad_dy == 0.f && rad_dz == 0.f) return;
	
	frustum.SetFront(
		math::Quat::RotateX(rad_dx) *
		math::Quat::RotateY(rad_dy) *
		math::Quat::RotateZ(rad_dz) *
		frustum.Front());
}

void RE_Camera::Orbit(float rad_dx, float rad_dy, math::vec center)
{
	if (rad_dx == 0.f && rad_dy == 0.f) return;

	math::vec front = math::Quat::RotateX(-rad_dx) * math::Quat::RotateY(-rad_dy) * frustum.Front();
	float distance = frustum.Pos().Distance(center);
	math::vec pos = center - (front * distance);

	frustum.SetFrame(pos, front, math::vec::unitY);
}

void RE_Camera::Focus(math::vec center, float radius, float min_dist)
{
	if (radius <= 0) return;

	float camDistance = min_dist;

	// Vertical distance
	float v_dist = radius / math::Sin(frustum.VerticalFov() / 2.0f);
	if (v_dist > camDistance) camDistance = v_dist;

	// Horizontal distance
	float h_dist = radius / math::Sin(frustum.HorizontalFov() / 2.0f);
	if (h_dist > camDistance) camDistance = h_dist;

	frustum.SetPos(center - (frustum.Front() * camDistance));
}

void RE_Camera::Focus(math::AABB box, float min_dist)
{
	Focus(box.CenterPoint(), box.HalfSize().Length(), min_dist);
}

#pragma endregion

#pragma region Setters / Getters

void RE_Camera::SetFOVRads(float vertical_fov_rads)
{
	frustum.SetPerspective(
		2.0f * math::Atan(math::Tan(vertical_fov_rads / 2.0f) * (bounds.x / bounds.y)),
		vertical_fov_rads);
}

void RE_Camera::SetFOVDegrees(float vertical_fov_degrees)
{
	SetFOVRads(RE_Math::Cap(vertical_fov_degrees, 1.f, 180.f) * RE_Math::deg_to_rad);
}

void RE_Camera::SetAspectRatio(AspectRatio aspect_ratio)
{
	ar = aspect_ratio;
	SetBounds(bounds);
}

void RE_Camera::SetPerspective(float default_vfov_degrees)
{
	frustum.SetVerticalFovAndAspectRatio(default_vfov_degrees * RE_Math::deg_to_rad, bounds.x / bounds.y);
}

math::float4 RE_Camera::GetTargetViewPort(int window_width, int window_height) const
{
	return GetTargetViewPort({ static_cast<float>(window_width), static_cast<float>(window_height) });
}

math::float4 RE_Camera::GetTargetViewPort(math::float2 window_bounds) const
{
	math::float4 viewPort = math::float4(0.f, 0.f, bounds.x, bounds.y);

	switch (ar)
	{
	case AspectRatio::Square_1x1: viewPort.x = (window_bounds.x / 2.f) - (viewPort.z / 2.f); break;
	case AspectRatio::TraditionalTV_4x3: viewPort.x = (window_bounds.x / 2.f) - (viewPort.z / 2.f); break;
	case AspectRatio::Movietone_16x9:
	{
		if (viewPort.w < window_bounds.y)
			viewPort.y = (window_bounds.y / 2.f) - (viewPort.w / 2.f);
		break;
	}
	default: break;
	}

	return viewPort;
}

#pragma endregion

#pragma region Skybox

void RE_Camera::UseSkybox() const { if (skyboxMD5) RE_RES->Use(skyboxMD5); }
void RE_Camera::UnUseSkybox() const { if (skyboxMD5) RE_RES->UnUse(skyboxMD5); }

#pragma endregion

#pragma region Serialization

void RE_Camera::JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->PushFloat2("bounds", bounds);
	node->Push("aspect_ratio", static_cast<uint>(ar));
	node->Push("v_fov_rads", GetVFOVRads());
	node->Push("near_plane", frustum.NearPlaneDistance());
	node->Push("far_plane", frustum.FarPlaneDistance());

	node->Push("usingSkybox", usingSkybox);
	node->Push("skyboxResource", (skyboxMD5) ? resources->at(skyboxMD5) : -1);

	DEL(node)
}

void RE_Camera::JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources)
{
	SetupFrustum(
		node->PullFloat2("bounds", { 300.f, 300.f }),
		static_cast<AspectRatio>(node->PullUInt("aspect_ratio", 0)),
		node->PullFloat("v_fov_rads", 0.523599f),
		node->PullFloat("near_plane", 1.0f),
		node->PullFloat("far_plane", 5000.0f));

	usingSkybox = node->PullBool("usingSkybox", true);
	int sbRes = node->PullInt("skyboxResource", -1);
	skyboxMD5 = (sbRes != -1) ? resources->at(sbRes) : nullptr;

	DEL(node)
}

size_t RE_Camera::GetBinarySize() const
{
	return sizeof(math::float2) + sizeof(uint) + sizeof(float) * 3 + sizeof(bool) + sizeof(int);
}

void RE_Camera::BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(math::float2);
	memcpy(cursor, &bounds, size);
	cursor += size;

	size = sizeof(uint);
	auto aspect_ratio = static_cast<uint>(ar);
	memcpy(cursor, &aspect_ratio, size);
	cursor += size;

	size = sizeof(float);
	float v_fov = GetVFOVRads();
	memcpy(cursor, &v_fov, size);
	cursor += size;

	float near_plane = frustum.NearPlaneDistance();
	memcpy(cursor, &near_plane, size);
	cursor += size;

	float far_plane = frustum.FarPlaneDistance();
	memcpy(cursor, &far_plane, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(cursor, &usingSkybox, size);
	cursor += size;

	size = sizeof(int);
	int sbres = (skyboxMD5) ? resources->at(skyboxMD5) : -1;
	memcpy(cursor, &sbres, size);
	cursor += size;
}

void RE_Camera::BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(math::float2);
	memcpy(&bounds, cursor, size);
	cursor += size;

	uint aspect_ratio;
	size = sizeof(uint);
	memcpy(&aspect_ratio, cursor, size);
	cursor += size;

	float v_fov;
	memcpy(&v_fov, cursor, size);
	cursor += size;

	float near_plane;
	size = sizeof(float);
	memcpy(&near_plane, cursor, size);
	cursor += size;

	float far_plane;
	memcpy(&far_plane, cursor, size);
	cursor += size;

	SetupFrustum(
		bounds,
		static_cast<AspectRatio>(aspect_ratio),
		v_fov,
		near_plane,
		far_plane);

	size = sizeof(bool);
	memcpy(&usingSkybox, cursor, size);
	cursor += size;

	size = sizeof(int);
	int sbRes = -1;
	memcpy(&sbRes, cursor, size);
	skyboxMD5 = (sbRes != -1) ? resources->at(sbRes) : nullptr;
	cursor += size;
}

#pragma endregion