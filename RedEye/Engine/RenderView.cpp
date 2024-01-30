#include "RenderView.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_Json.h"
#include "RenderedWindow.h"

#include <ImGui/imgui.h>

RenderView::RenderView(uint default_fbo, uint deferred_fbo, RenderSettings settings, math::float4 clipDistance) :
	settings(settings), clip_distance(clipDistance)
{
	fbos[0] = default_fbo;
	fbos[1] = deferred_fbo;
}

void RenderView::DrawEditor(const char* _id)
{
	eastl::string id = _id;
	settings.DrawEditor(_id);

	ImGui::Separator();

	// Clear Color
	ImGui::PushID((id + "Clear Color").c_str());
	ImGui::ColorEdit4("Clear Color", clear_color.ptr(), ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
	ImGui::PopID();

	// Clip Distance
	if (!settings.HasFlag(RenderSettings::Flag::CLIP_DISTANCE))
		return;

	ImGui::PushID((id + "Clip Distance").c_str());
	ImGui::DragFloat4("Clip Distance", clip_distance.ptr(), 0.1f);
	ImGui::PopID();
}

uint RenderView::GetFBO() const
{
	return fbos[settings.light == RenderSettings::LightMode::DEFERRED];
}

void RenderView::Load(RE_Json* node)
{
	if (node == nullptr) return;

	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::RenderView)

	settings.Load(node->PullJObject("Settings"));
	clear_color = node->PullFloat4("Clear Color", { 0.0f, 0.0f, 0.0f, 1.0f });
	clip_distance = node->PullFloat4("Clip Distance", math::float4::zero);

	DEL(node)
}

void RenderView::Save(RE_Json* node) const
{
	if (node == nullptr) return;

	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::RenderView)
		
	settings.Save(node->PushJObject("Settings"));
	node->PushFloat4("Clear Color", clear_color);
	node->PushFloat4("Clip Distance", clip_distance);

	DEL(node)
}