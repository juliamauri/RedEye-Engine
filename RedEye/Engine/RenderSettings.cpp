#include "RenderSettings.h"

#include "RE_Memory.h"
#include "RE_Json.h"

#include <EASTL/string.h>
#include <ImGui/imgui.h>

void RenderSettings::DrawEditor(const char* _id)
{
	eastl::string id = _id;

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

	ImGui::Separator();

	// Light
	auto light_int = static_cast<int>(light);
	ImGui::PushID((id + " Light Mode").c_str());
	if (ImGui::Combo("Light Mode", &light_int, "LIGHT_DISABLED\0LIGHT_GL\0LIGHT_DIRECT\0LIGHT_DEFERRED\0"))
		light = LightMode(light_int);
	ImGui::PopID();
}

void RenderSettings::CheckboxFlag(const char* label, Flag flag)
{
	bool flag_state = HasFlag(flag);
	if (ImGui::Checkbox(label, &flag_state))
		flag_state ? AddFlag(flag) : RemoveFlag(flag);
}

void RenderSettings::Load(RE_Json* node)
{
	flags = static_cast<ushort>(node->PullUInt("Flags", 0));
	light = static_cast<LightMode>(node->PullUInt("Light Mode", static_cast<uint>(light)));
	DEL(node)
}

void RenderSettings::Save(RE_Json* node) const
{
	node->Push("Flags", static_cast<uint>(flags));
	node->Push("Light Mode", static_cast<uint>(light));
	DEL(node)
}
