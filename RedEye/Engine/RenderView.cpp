#include "RenderView.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_Json.h"
#include "RE_CameraManager.h"
#include <ImGui/imgui.h>

RenderView::~RenderView()
{
	DEL(camera)
}

void RenderView::DrawEditor()
{
	if (ImGui::TreeNodeEx((name + " View").c_str(),
		ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		auto light_int = static_cast<int>(light);
		ImGui::PushID(eastl::string("#light" + name).c_str());
		if (ImGui::Combo("Light Mode", &light_int, "LIGHT_DISABLED\0LIGHT_GL\0LIGHT_DIRECT\0LIGHT_DEFERRED\0"))
			light = RenderView::LightMode(light_int);
		ImGui::PopID();

		ImGui::PushID(eastl::string("#clearcolor" + name).c_str());
		ImGui::ColorEdit4("Clear Color", clear_color.ptr(), ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
		ImGui::PopID();

		static const char* flag_labels[12] = {
			"Fustrum Culling", "Override Culling", "Outline Selection", "Debug Draw", "Skybox", "Blending",
			"Wireframe", "Face Culling", "Texture 2D", "Color Material", "Depth Testing", "Clipping Distance" };

		for (ushort count = 0; count < 12; ++count)
		{
			bool temp = flags & (1 << count);
			ImGui::PushID(eastl::string(flag_labels[count] + name).c_str());
			if (ImGui::Checkbox(flag_labels[count], &temp))
				temp ? flags |= (1 << count) : flags -= (1 << count);
			ImGui::PopID();
		}

		if (HasFlag(Flag::CLIP_DISTANCE))
		{
			ImGui::PushID(eastl::string("clipdistance" + name).c_str());
			ImGui::DragFloat4("Clip Distance", clip_distance.ptr(), 0.1f);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}
}

const math::Frustum* RenderView::GetFrustum() const
{
	if (!HasFlag(RenderView::Flag::FRUSTUM_CULLING)) return nullptr;
	if (HasFlag(RenderView::Flag::OVERRIDE_CULLING)) return RE_CameraManager::GetCullingFrustum();
	return &camera->GetFrustum();
}

void RenderView::Save(RE_Json* node) const
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::RenderView);

	if (node == nullptr) return;

	node->Push("Light Mode", static_cast<uint>(light));
	node->PushFloat4("Clear Color", clear_color);
	node->PushFloat4("Clip Distance", clip_distance);

	node->Push("Flags", static_cast<uint>(flags));

	DEL(node)
}

void RenderView::Load(RE_Json* node)
{
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::RenderView);

	if (node == nullptr) return;

	light = static_cast<LightMode>(node->PullUInt("Light Mode", static_cast<uint>(LightMode::DEFERRED)));
	clear_color = node->PullFloat4("Clear Color", { 0.0f, 0.0f, 0.0f, 1.0f });
	clip_distance = node->PullFloat4("Clip Distance", math::float4::zero);

	flags = static_cast<ushort>(node->PullUInt("Flags", 0));

	DEL(node)
}