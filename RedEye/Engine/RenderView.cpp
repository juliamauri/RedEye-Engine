#include "RenderView.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_Json.h"
#include "RenderedWindow.h"

#include <ImGui/imgui.h>

void RenderView::DrawEditor(const char* _id)
{
	eastl::string id = _id;

	// Light
	auto light_int = static_cast<int>(light_mode);
	ImGui::PushID((id + " Light Mode").c_str());
	if (ImGui::Combo("Light Mode", &light_int, "LIGHT_DISABLED\0LIGHT_GL\0LIGHT_DIRECT\0LIGHT_DEFERRED\0"))
		light_mode = RenderView::LightMode(light_int);
	ImGui::PopID();

	// Clear Color
	ImGui::PushID((id + "Clear Color").c_str());
	ImGui::ColorEdit4("Clear Color", clear_color.ptr(), ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
	ImGui::PopID();

	// Clip Distance
	if (HasFlag(Flag::CLIP_DISTANCE))
	{
		ImGui::PushID((id + "Clip Distance").c_str());
		ImGui::DragFloat4("Clip Distance", clip_distance.ptr(), 0.1f);
		ImGui::PopID();
	}

	// Flags
	static const char* flag_labels[12] = {
		"Fustrum Culling", "Override Culling", "Outline Selection", "Debug Draw", "Skybox", "Blending",
		"Wireframe", "Face Culling", "Texture 2D", "Color Material", "Depth Testing", "Clipping Distance" };
	for (ushort count = 0; count < 12; ++count)
	{
		bool temp = flags & (1 << count);
		ImGui::PushID((id + flag_labels[count]).c_str());
		if (ImGui::Checkbox(flag_labels[count], &temp))
			temp ? flags |= (1 << count) : flags -= (1 << count);
		ImGui::PopID();
	}
}

/*const math::Frustum* RenderView::GetFrustum() const
{
	if (!HasFlag(RenderView::Flag::FRUSTUM_CULLING)) return nullptr;
	if (HasFlag(RenderView::Flag::OVERRIDE_CULLING)) return RE_CameraManager::GetCullingFrustum();
	return &camera->GetFrustum();
}*/

void RenderView::CheckboxFlag(const char* label, Flag flag)
{
	bool tmp = HasFlag(flag);
	if (ImGui::Checkbox(label, &tmp))
		tmp ? AddFlag(flag) : RemoveFlag(flag);
}

void RenderView::Load(RE_Json* node)
{
	if (node == nullptr) return;

	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::RenderView)

	flags = static_cast<ushort>(node->PullUInt("Flags", 0));
	light_mode = static_cast<LightMode>(node->PullUInt("Light Mode", static_cast<uint>(LightMode::DEFERRED)));
	clear_color = node->PullFloat4("Clear Color", { 0.0f, 0.0f, 0.0f, 1.0f });
	clip_distance = node->PullFloat4("Clip Distance", math::float4::zero);

	DEL(node)
}

void RenderView::Save(RE_Json* node) const
{
	if (node == nullptr) return;

	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::RenderView)

	node->Push("Flags", static_cast<uint>(flags));
	node->Push("Light Mode", static_cast<uint>(light_mode));
	node->PushFloat4("Clear Color", clear_color);
	node->PushFloat4("Clip Distance", clip_distance);

	DEL(node)
}