#include "RenderView.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_Json.h"
#include <ImGui/imgui.h>

RenderView::RenderView(
	eastl::string name,
	eastl::pair<unsigned int, unsigned int> fbos,
	short flags,
	LightMode light,
	math::float4 clipDistance) :

	name(name), fbos(fbos), flags(flags), light(light), clip_distance(clipDistance),
	clear_color({ 0.0f, 0.0f, 0.0f, 1.0f })
{}

const unsigned int RenderView::GetFBO() const { return light != LightMode::DEFERRED ? fbos.first : fbos.second; }

inline const bool RenderView::HasFlag(Flag flag) const { return flags & static_cast<const short>(flag); }

void RenderView::DrawEditor()
{
	if (ImGui::TreeNodeEx((name + " View").c_str(), ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		auto light_int = static_cast<int>(light);
		ImGui::PushID(eastl::string("#light" + name).c_str());
		if (ImGui::Combo("Light Mode", &light_int, "LIGHT_DISABLED\0LIGHT_GL\0LIGHT_DIRECT\0LIGHT_DEFERRED\0"))
			light = RenderView::LightMode(light_int);
		ImGui::PopID();

		ImGui::PushID(eastl::string("#clearcolor" + name).c_str());
		ImGui::ColorEdit4("Clear Color", clear_color.ptr(), ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
		ImGui::PopID();

		for (ushort count = 0; count < 12; ++count)
		{
			bool temp = flags & (1 << count);
			ImGui::PushID(eastl::string(RenderView::flag_labels[count] + name).c_str());
			if (ImGui::Checkbox(RenderView::flag_labels[count], &temp))
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

void RenderView::Load(RE_Json* node)
{
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::RenderView);

	if (node == nullptr) return;

	light = static_cast<LightMode>(node->PullUInt("Light Mode", static_cast<uint>(LightMode::DEFERRED)));
	clear_color = node->PullFloat4("Clear Color", { 0.0f, 0.0f, 0.0f, 1.0f });
	clip_distance = node->PullFloat4("Clip Distance", math::float4::zero);

	for (int i = 0; i < 12; i++)
	{
		int flag_id = 1 << i;
		if (node->PullBool(flag_labels[i], flags & flag_id)) flags |= flag_id;
		else flags -= flag_id;
	}

	DEL(node);
}

void RenderView::Save(RE_Json* node) const
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::RenderView);

	if (node == nullptr) return;

	node->Push("Light Mode", static_cast<uint>(light));
	node->PushFloat4("Clear Color", clear_color);
	node->PushFloat4("Clip Distance", clip_distance);

	for (int i = 0; i < 12; i++) node->Push(flag_labels[i], static_cast<bool>(flags & (1 << i)));

	DEL(node);
}
