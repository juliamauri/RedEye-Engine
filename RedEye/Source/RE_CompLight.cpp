#include "RE_CompLight.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_ShaderImporter.h"
#include "RE_FileSystem.h"
#include "RE_GOManager.h"
#include "ImGui\imgui.h"

RE_CompLight::RE_CompLight() : RE_Component(C_LIGHT)
{
	diffuse = math::vec(0.8f);
	cutOff[0] = 12.5f;
	outerCutOff[0] = 17.5f;
	UpdateCutOff();
}

RE_CompLight::~RE_CompLight()
{}

void RE_CompLight::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent)
{
	pool_gos = pool;
	if (go = parent) pool_gos->AtPtr(go)->ReportComponent(id, C_LIGHT);

	RE_CompLight* cmpLight = dynamic_cast<RE_CompLight*>(copy);
	type = cmpLight->type;
	intensity = cmpLight->intensity;
	constant = cmpLight->constant;
	linear = cmpLight->linear;
	quadratic = cmpLight->quadratic;
	diffuse = cmpLight->diffuse;
	specular = cmpLight->specular;

	cutOff[0] = cmpLight->cutOff[0];
	outerCutOff[0] = cmpLight->outerCutOff[0];
	UpdateCutOff();
}

void RE_CompLight::CallShaderUniforms(unsigned int shader, const char* name) const
{
	eastl::string unif_name = name;

	RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "type").c_str()), float(type));
	RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "intensity").c_str()), intensity);
	RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "diffuse").c_str()), diffuse);
	RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "specular").c_str()), specular);

	RE_CompTransform* transform = GetGOPtr()->GetTransformPtr();
	RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "direction").c_str()), transform->GetFront());

	if (type != L_DIRECTIONAL)
	{
		RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "position").c_str()), transform->GetGlobalPosition());
		RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "constant").c_str()), constant);
		RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "linear").c_str()), linear);
		RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "quadratic").c_str()), quadratic);

		if (type == L_SPOTLIGHT)
		{
			RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "cutOff").c_str()), cutOff[1]);
			RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "outerCutOff").c_str()), outerCutOff[1]);
		}
	}
}

void RE_CompLight::DrawProperties()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::Combo("Light Type", (int*)&type, "DIRECTIONAL\0POINT\0SPOTLIGHT");

		ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 50.0f, "%.2f");
		ImGui::DragFloat("Constant", &constant, 0.01f, 0.001f, 5.0f, "%.2f");
		ImGui::DragFloat("Linear", &linear, 0.01f, 0.001f, 5.0f, "%.3f");
		ImGui::DragFloat("Quadratic", &quadratic, 0.001f, 0.001f, 5.0f, "%.3f");

		ImGui::ColorEdit3("Diffuse Color", &diffuse[0]);
		ImGui::DragFloat("Specular", &specular, 0.01f, 0.f, 1.f, "%.2f");

		if (type == L_SPOTLIGHT)
		{
			if (ImGui::DragFloat("CutOff", &cutOff[0], 1.0f, 0.0f, 90.0f, "%.0f"))
				cutOff[1] = math::Cos(math::DegToRad(cutOff[0]));
			if (ImGui::DragFloat("OuterCutOff", &outerCutOff[0], 1.0f, 0.0f, 90.0f, "%.0f"))
				outerCutOff[1] = math::Cos(math::DegToRad(outerCutOff[0]));
		}
	}
}

void RE_CompLight::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const
{
	node->PushInt("type", type);
	node->PushFloat("intensity", intensity);
	node->PushFloat("constant", constant);
	node->PushFloat("linear", linear);
	node->PushFloat("quadratic", quadratic);
	node->PushFloatVector("diffuse", diffuse);
	node->PushFloat("specular", specular);
	node->PushFloat("cutOff", cutOff[0]);
	node->PushFloat("outerCutOff", outerCutOff[0]);
}

void RE_CompLight::DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources)
{
	type = static_cast<LightType>(node->PullInt("type", L_POINT));

	intensity = node->PullFloat("intensity", intensity);
	constant = node->PullFloat("constant", constant);
	linear = node->PullFloat("linear", linear);
	quadratic = node->PullFloat("quadratic", quadratic);
	diffuse = node->PullFloatVector("diffuse", diffuse);
	specular = node->PullFloat("specular", specular);
	cutOff[0] = node->PullFloat("cutOff", cutOff[0]);
	outerCutOff[0] = node->PullFloat("outerCutOff", outerCutOff[0]);
	UpdateCutOff();
}

unsigned int RE_CompLight::GetBinarySize() const
{
	return sizeof(int) + sizeof(float) * 10;
}

void RE_CompLight::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &intensity, size);
	cursor += size;
	memcpy(cursor, &constant, size);
	cursor += size;
	memcpy(cursor, &linear, size);
	cursor += size;
	memcpy(cursor, &quadratic, size);
	cursor += size;
	memcpy(cursor, diffuse.ptr(), size * 3);
	cursor += size * 3;
	memcpy(cursor, &specular, size);
	cursor += size;
	memcpy(cursor, &cutOff[0], size);
	cursor += size;
	memcpy(cursor, &outerCutOff[0], size);
	cursor += size;
}

void RE_CompLight::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;

	size = sizeof(float);
	memcpy(&intensity, cursor, size);
	cursor += size;
	memcpy(&constant, cursor, size);
	cursor += size;
	memcpy(&linear, cursor, size);
	cursor += size;
	memcpy(&quadratic, cursor, size);
	cursor += size;
	memcpy(&diffuse[0], cursor, size * 3);
	cursor += size * 3;
	memcpy(&specular, cursor, size);
	cursor += size;
	memcpy(&cutOff[0], cursor, size);
	cursor += size;
	memcpy(&outerCutOff[0], cursor, size);
	cursor += size;

	UpdateCutOff();
}

inline void RE_CompLight::UpdateCutOff()
{
	cutOff[1] = math::Cos(math::DegToRad(cutOff[0]));
	outerCutOff[1] = math::Cos(math::DegToRad(outerCutOff[0]));
}
