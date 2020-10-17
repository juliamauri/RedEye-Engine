#include "RE_CompLight.h"

#include "RE_GameObject.h"
#include "ImGui\imgui.h"

RE_CompLight::RE_CompLight() :RE_Component(C_LIGHT, nullptr)
{}

RE_CompLight::~RE_CompLight()
{}

void RE_CompLight::SetUp(RE_GameObject* parent)
{
	go = parent;
	parent->AddComponent(this);
}

void RE_CompLight::SetUp(const RE_CompLight& cmpLight, RE_GameObject* parent)
{
	go = parent;
	parent->AddComponent(this);
}

LightType RE_CompLight::GetType() const
{
	return type;
}

void RE_CompLight::DrawProperties()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::Combo("Light Type", (int*)&type, "DIRECTIONAL\0POINT\0SPOTLIGHT");

		ImGui::DragFloat("Intensity", &intensity, 1.0f, 0.0f, 5.0f, "%.1f");
		ImGui::DragFloat("Constant", &constant, 1.0f, 0.0f, 5.0f, "%.1f");
		ImGui::DragFloat("Linear", &linear, 1.0f, 0.0f, 5.0f, "%.1f");
		ImGui::DragFloat("Quadratic", &quadratic, 1.0f, 0.0f, 5.0f, "%.1f");

		ImGui::ColorEdit3("Ambient Color", &ambient[0]);
		ImGui::ColorEdit3("Diffuse Color", &diffuse[0]);
		ImGui::ColorEdit3("Specular Color", &specular[0]);

		if (type == L_SPOTLIGHT)
		{
			ImGui::DragFloat("CutOff", &cutOff, 1.0f, 0.0f, 5.0f, "%.1f");
			ImGui::DragFloat("OuterCutOff", &outerCutOff, 1.0f, 0.0f, 5.0f, "%.1f");
		}
	}
}

void RE_CompLight::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	//node->PushBool("isPrespective", isPerspective);
}

void RE_CompLight::DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources, RE_GameObject* parent)
{
	//usingSkybox = node->PullBool("usingSkybox", true);
}

unsigned int RE_CompLight::GetBinarySize() const
{
	return 0; // sizeof(bool) * 3 + sizeof(int) * 2 + sizeof(float) * 3;
}

void RE_CompLight::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	/*size_t size = sizeof(bool);
	memcpy(cursor, &isPerspective, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &near_plane, size);
	cursor += size;*/
}

void RE_CompLight::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources, RE_GameObject* parent)
{
	/*size_t size = sizeof(bool);
	memcpy(&isPerspective, cursor, size);
	cursor += size;

	size = sizeof(float);
	memcpy(&near_plane, cursor, size);
	cursor += size;*/
}
