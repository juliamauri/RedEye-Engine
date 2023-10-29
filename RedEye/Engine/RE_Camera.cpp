#include "RE_Camera.h"
#include "RE_Math.h"

#include "RE_Memory.h"
#include "Application.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_SkyBox.h"
#include "RE_Json.h"

#include "ModuleWindow.h"

#include <ImGui/imgui.h>
#include <SDL2/SDL_opengl.h>

void RE_Camera::SetupFrustum(float n_plane, float f_plane, AspectRatio ar, float v_fov)
{
	frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);
	frustum.SetViewPlaneDistances(n_plane, f_plane);
	SetAspectRatio(ar, false);

	if (v_fov > 0.f) frustum.SetVerticalFovAndAspectRatio(v_fov, width / height);
	else frustum.SetOrthographic(width, height);
}

void RE_Camera::SetFrame(const math::vec& pos, const math::vec& front, const math::vec& up)
{
	frustum.SetFrame(pos, front, up);
}

#pragma region Draws

bool RE_Camera::DrawProperties()
{
	bool ret = false; // return true if camera AR is modified

	float near_plane = frustum.NearPlaneDistance();
	float far_plane = frustum.FarPlaneDistance();
	if (ImGui::DragFloat("Near Plane", &near_plane, 1.0f, 1.0f, far_plane, "%.1f")) SetPlanesDistance(near_plane, far_plane);
	if (ImGui::DragFloat("Far Plane", &far_plane, 10.0f, near_plane, 65000.0f, "%.1f")) SetPlanesDistance(near_plane, far_plane);

	auto aspect_ratio = static_cast<int>(target_ar);
	if (ImGui::Combo("Aspect Ratio", &aspect_ratio, "Fit Window\0Square 1x1\0TraditionalTV 4x3\0Movietone 16x9\0Personalized\0"))
	{
		target_ar = static_cast<AspectRatio>(aspect_ratio);
		ret = true;
	}

	// TODO Rub: Custom Aspect Ratio
	//if (target_ar == Custom)
	//{
	//	int w = width;
	//	int h = height;
	//	if (ImGui::DragInt("Width", &w, 4.0f, 1, App->window->GetWidth(), "%.1f") ||
	//		ImGui::DragInt("Height", &h, 4.0f, 1, App->window->GetMaxHeight(), "%.1f"))
	//		SetBounds(w,h);
	//}

	int current_type = IsPerspective();
	if (ImGui::Combo("Type", &current_type, "Perspective\0Orthographic\0"))
		SwapCameraType();


	auto v_fov_degrees = frustum.VerticalFov() * RE_Math::rad_to_deg;
	if (IsPerspective() && ImGui::DragFloat("FOV", &v_fov_degrees, 1.0f, 0.0f, 180.0f, "%.1f"))
		SetFOVDegrees(v_fov_degrees);


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

	return ret;
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

	//rot_eul -= math::vec(rad_dy * -1, rad_dx, rad_dz);
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
		2.0f * math::Atan(math::Tan(vertical_fov_rads / 2.0f) * (width / height)),
		vertical_fov_rads);
}

void RE_Camera::SetFOVDegrees(float vertical_fov_degrees)
{
	SetFOVRads(RE_Math::Cap(vertical_fov_degrees, 1.f, 180.f) * RE_Math::deg_to_rad);
}

void RE_Camera::SetAspectRatio(AspectRatio aspect_ratio, bool apply_changes)
{
	target_ar = aspect_ratio;
	SetBounds(static_cast<float>(RE_WINDOW->GetWidth()), static_cast<float>(RE_WINDOW->GetHeight()), apply_changes);
}

void RE_Camera::SetBounds(float w, float h, bool apply_changes)
{
	switch (target_ar)
	{
	case AspectRatio::Fit_Window:

		width = w;
		height = h;
		break;

	case AspectRatio::Square_1x1:

		if (w >= h) width = height = h;
		else width = height = w;
		break;

	case AspectRatio::TraditionalTV_4x3:

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

	case AspectRatio::Movietone_16x9:

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

	default:

		width = w;
		height = h;
		break;
	}
	if (!apply_changes) return;
	if (IsPerspective()) SetFOVRads(frustum.VerticalFov());
	else frustum.SetOrthographic(width, height);
}

void RE_Camera::SetPerspective(float default_vfov_degrees)
{
	frustum.SetVerticalFovAndAspectRatio(default_vfov_degrees * RE_Math::deg_to_rad, width / height);
}

void RE_Camera::SetOrthographic()
{
	frustum.SetOrthographic(width, height);
}

void RE_Camera::SwapCameraType()
{
	IsPerspective() ? SetOrthographic() : SetPerspective();
}

void RE_Camera::GetTargetViewPort(math::float4& viewPort) const
{
	viewPort.z = width;
	viewPort.w = height;

	float wH = static_cast<float>(RE_WINDOW->GetHeight());
	float wW = static_cast<float>(RE_WINDOW->GetWidth());

	switch (target_ar)
	{
	case AspectRatio::Fit_Window:
	{
		viewPort.x = 0.f;
		viewPort.y = 0.f;
		break;
	}
	case AspectRatio::Square_1x1:
	{
		viewPort.y = 0.f;
		viewPort.x = (wW / 2.f) - (width / 2.f);
		break;
	}
	case AspectRatio::TraditionalTV_4x3:
	{
		viewPort.y = 0.f;
		viewPort.x = (wW / 2.f) - (width / 2.f);
		break;
	}
	case AspectRatio::Movietone_16x9:
	{
		viewPort.x = 0.f;
		if (height < wH) viewPort.y = (wH / 2.f) - (height / 2.f);
		break;
	}
	default:
	{
		viewPort.x = 0.f;
		viewPort.y = 0.f;
		break;
	}
	}
}

#pragma endregion

#pragma region Skybox

void RE_Camera::UseSkybox() const { if (skyboxMD5) RE_RES->Use(skyboxMD5); }
void RE_Camera::UnUseSkybox() const { if (skyboxMD5) RE_RES->UnUse(skyboxMD5); }

#pragma endregion

#pragma region Serialization

void RE_Camera::JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->Push("near_plane", frustum.NearPlaneDistance());
	node->Push("far_plane", frustum.FarPlaneDistance());
	node->Push("v_fov_rads", GetVFOVRads());
	node->Push("aspect_ratio", static_cast<uint>(target_ar));

	node->Push("usingSkybox", usingSkybox);
	node->Push("skyboxResource", (skyboxMD5) ? resources->at(skyboxMD5) : -1);

	DEL(node)
}

void RE_Camera::JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources)
{
	SetupFrustum(
		node->PullFloat("near_plane", 1.0f),
		node->PullFloat("far_plane", 5000.0f),
		static_cast<RE_Camera::AspectRatio>(node->PullUInt("aspect_ratio", 0)),
		node->PullFloat("v_fov_rads", 0.523599f));

	usingSkybox = node->PullBool("usingSkybox", true);
	int sbRes = node->PullInt("skyboxResource", -1);
	skyboxMD5 = (sbRes != -1) ? resources->at(sbRes) : nullptr;

	DEL(node)
}

size_t RE_Camera::GetBinarySize() const
{
	return sizeof(float) * 3 + sizeof(uint) + sizeof(bool) + sizeof(int);
}

void RE_Camera::BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const
{
	float near_plane = frustum.NearPlaneDistance();
	float far_plane = frustum.FarPlaneDistance();
	float v_fov = GetVFOVRads();
	auto aspect_ratio = static_cast<uint>(target_ar);

	size_t size = sizeof(float);
	memcpy(cursor, &near_plane, size);
	cursor += size;

	memcpy(cursor, &far_plane, size);
	cursor += size;

	memcpy(cursor, &v_fov, size);
	cursor += size;

	size = sizeof(uint);
	memcpy(cursor, &aspect_ratio, size);
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
	float near_plane;
	size_t size = sizeof(float);
	memcpy(&near_plane, cursor, size);
	cursor += size;

	float far_plane;
	memcpy(&far_plane, cursor, size);
	cursor += size;

	float v_fov;
	memcpy(&v_fov, cursor, size);
	cursor += size;

	uint aspect_ratio;
	size = sizeof(uint);
	memcpy(&aspect_ratio, cursor, size);
	cursor += size;

	SetupFrustum(near_plane, far_plane, static_cast<AspectRatio>(aspect_ratio), v_fov);

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